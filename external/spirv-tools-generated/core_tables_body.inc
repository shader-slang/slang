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
    case Extension::kSPV_INTEL_subgroup_buffer_prefetch: return IR(3325, 35);
    case Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate: return IR(3360, 46);
    case Extension::kSPV_INTEL_subgroups: return IR(3406, 20);
    case Extension::kSPV_INTEL_task_sequence: return IR(3426, 24);
    case Extension::kSPV_INTEL_tensor_float32_conversion: return IR(3450, 36);
    case Extension::kSPV_INTEL_ternary_bitwise_function: return IR(3486, 35);
    case Extension::kSPV_INTEL_unstructured_loop_controls: return IR(3521, 37);
    case Extension::kSPV_INTEL_usm_storage_classes: return IR(3558, 30);
    case Extension::kSPV_INTEL_variable_length_array: return IR(3588, 32);
    case Extension::kSPV_INTEL_vector_compute: return IR(3620, 25);
    case Extension::kSPV_KHR_16bit_storage: return IR(3645, 22);
    case Extension::kSPV_KHR_8bit_storage: return IR(3667, 21);
    case Extension::kSPV_KHR_abort: return IR(3688, 14);
    case Extension::kSPV_KHR_bfloat16: return IR(3702, 17);
    case Extension::kSPV_KHR_bit_instructions: return IR(3719, 25);
    case Extension::kSPV_KHR_compute_shader_derivatives: return IR(3744, 35);
    case Extension::kSPV_KHR_constant_data: return IR(3779, 22);
    case Extension::kSPV_KHR_cooperative_matrix: return IR(3801, 27);
    case Extension::kSPV_KHR_device_group: return IR(3828, 21);
    case Extension::kSPV_KHR_expect_assume: return IR(3849, 22);
    case Extension::kSPV_KHR_float_controls: return IR(3871, 23);
    case Extension::kSPV_KHR_float_controls2: return IR(3894, 24);
    case Extension::kSPV_KHR_fma: return IR(3918, 12);
    case Extension::kSPV_KHR_fragment_shader_barycentric: return IR(3930, 36);
    case Extension::kSPV_KHR_fragment_shading_rate: return IR(3966, 30);
    case Extension::kSPV_KHR_integer_dot_product: return IR(3996, 28);
    case Extension::kSPV_KHR_linkonce_odr: return IR(4024, 21);
    case Extension::kSPV_KHR_maximal_reconvergence: return IR(4045, 30);
    case Extension::kSPV_KHR_multiview: return IR(4075, 18);
    case Extension::kSPV_KHR_no_integer_wrap_decoration: return IR(4093, 35);
    case Extension::kSPV_KHR_non_semantic_info: return IR(4128, 26);
    case Extension::kSPV_KHR_opacity_micromap: return IR(4154, 25);
    case Extension::kSPV_KHR_physical_storage_buffer: return IR(4179, 32);
    case Extension::kSPV_KHR_poison_freeze: return IR(4211, 22);
    case Extension::kSPV_KHR_post_depth_coverage: return IR(4233, 28);
    case Extension::kSPV_KHR_quad_control: return IR(4261, 21);
    case Extension::kSPV_KHR_ray_cull_mask: return IR(4282, 22);
    case Extension::kSPV_KHR_ray_query: return IR(4304, 18);
    case Extension::kSPV_KHR_ray_tracing: return IR(4322, 20);
    case Extension::kSPV_KHR_ray_tracing_position_fetch: return IR(4342, 35);
    case Extension::kSPV_KHR_relaxed_extended_instruction: return IR(4377, 37);
    case Extension::kSPV_KHR_shader_atomic_counter_ops: return IR(4414, 34);
    case Extension::kSPV_KHR_shader_ballot: return IR(4448, 22);
    case Extension::kSPV_KHR_shader_clock: return IR(4470, 21);
    case Extension::kSPV_KHR_shader_draw_parameters: return IR(4491, 31);
    case Extension::kSPV_KHR_storage_buffer_storage_class: return IR(4522, 37);
    case Extension::kSPV_KHR_subgroup_rotate: return IR(4559, 24);
    case Extension::kSPV_KHR_subgroup_uniform_control_flow: return IR(4583, 38);
    case Extension::kSPV_KHR_subgroup_vote: return IR(4621, 22);
    case Extension::kSPV_KHR_terminate_invocation: return IR(4643, 29);
    case Extension::kSPV_KHR_uniform_group_instructions: return IR(4672, 35);
    case Extension::kSPV_KHR_untyped_pointers: return IR(4707, 25);
    case Extension::kSPV_KHR_variable_pointers: return IR(4732, 26);
    case Extension::kSPV_KHR_vulkan_memory_model: return IR(4758, 28);
    case Extension::kSPV_KHR_workgroup_memory_explicit_layout: return IR(4786, 41);
    case Extension::kSPV_NVX_multiview_per_view_attributes: return IR(4827, 38);
    case Extension::kSPV_NV_bindless_texture: return IR(4865, 24);
    case Extension::kSPV_NV_cluster_acceleration_structure: return IR(4889, 38);
    case Extension::kSPV_NV_compute_shader_derivatives: return IR(4927, 34);
    case Extension::kSPV_NV_cooperative_matrix: return IR(4961, 26);
    case Extension::kSPV_NV_cooperative_matrix2: return IR(4987, 27);
    case Extension::kSPV_NV_cooperative_matrix_decode_vector: return IR(5014, 40);
    case Extension::kSPV_NV_cooperative_vector: return IR(5054, 26);
    case Extension::kSPV_NV_displacement_micromap: return IR(5080, 29);
    case Extension::kSPV_NV_fragment_shader_barycentric: return IR(5109, 35);
    case Extension::kSPV_NV_geometry_shader_passthrough: return IR(5144, 35);
    case Extension::kSPV_NV_linear_swept_spheres: return IR(5179, 28);
    case Extension::kSPV_NV_mesh_shader: return IR(5207, 19);
    case Extension::kSPV_NV_push_constant_bank: return IR(5226, 26);
    case Extension::kSPV_NV_raw_access_chains: return IR(5252, 25);
    case Extension::kSPV_NV_ray_tracing: return IR(5277, 19);
    case Extension::kSPV_NV_ray_tracing_motion_blur: return IR(5296, 31);
    case Extension::kSPV_NV_sample_mask_override_coverage: return IR(5327, 37);
    case Extension::kSPV_NV_shader_atomic_fp16_vector: return IR(5364, 33);
    case Extension::kSPV_NV_shader_image_footprint: return IR(5397, 30);
    case Extension::kSPV_NV_shader_invocation_reorder: return IR(5427, 33);
    case Extension::kSPV_NV_shader_sm_builtins: return IR(5460, 26);
    case Extension::kSPV_NV_shader_subgroup_partitioned: return IR(5486, 35);
    case Extension::kSPV_NV_shading_rate: return IR(5521, 20);
    case Extension::kSPV_NV_stereo_view_rendering: return IR(5541, 29);
    case Extension::kSPV_NV_tensor_addressing: return IR(5570, 25);
    case Extension::kSPV_NV_viewport_array2: return IR(5595, 23);
    case Extension::kSPV_QCOM_bfloat16_muladd: return IR(5618, 25);
    case Extension::kSPV_QCOM_cooperative_matrix_conversion: return IR(5643, 39);
    case Extension::kSPV_QCOM_image_processing: return IR(5682, 26);
    case Extension::kSPV_QCOM_image_processing2: return IR(5708, 27);
    case Extension::kSPV_QCOM_image_processing3: return IR(5735, 27);
    case Extension::kSPV_QCOM_multiple_wait_queues: return IR(5762, 30);
    case Extension::kSPV_QCOM_subgroup_size: return IR(5792, 23);
    case Extension::kSPV_QCOM_tile_shading: return IR(5815, 22);
    case Extension::kSPV_SAMSUNG_intrinsic: return IR(5837, 22);
    case Extension::kSPV_VALIDATOR_ignore_type_decl_unique: return IR(5859, 38);
    case Extension::kSPV_VALVE_mixed_float_dot_product: return IR(5897, 34);
    default: break;
  }
  return {};
}

// Extension names and values, ordered by name
// The fields in order are:
//   name, indexing into kStrings
//   enum value
static const std::array<NameValue,205> kExtensionNames{{
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
    {IR(3325, 35), static_cast<uint32_t>(kSPV_INTEL_subgroup_buffer_prefetch)},
    {IR(3360, 46), static_cast<uint32_t>(kSPV_INTEL_subgroup_matrix_multiply_accumulate)},
    {IR(3406, 20), static_cast<uint32_t>(kSPV_INTEL_subgroups)},
    {IR(3426, 24), static_cast<uint32_t>(kSPV_INTEL_task_sequence)},
    {IR(3450, 36), static_cast<uint32_t>(kSPV_INTEL_tensor_float32_conversion)},
    {IR(3486, 35), static_cast<uint32_t>(kSPV_INTEL_ternary_bitwise_function)},
    {IR(3521, 37), static_cast<uint32_t>(kSPV_INTEL_unstructured_loop_controls)},
    {IR(3558, 30), static_cast<uint32_t>(kSPV_INTEL_usm_storage_classes)},
    {IR(3588, 32), static_cast<uint32_t>(kSPV_INTEL_variable_length_array)},
    {IR(3620, 25), static_cast<uint32_t>(kSPV_INTEL_vector_compute)},
    {IR(3645, 22), static_cast<uint32_t>(kSPV_KHR_16bit_storage)},
    {IR(3667, 21), static_cast<uint32_t>(kSPV_KHR_8bit_storage)},
    {IR(3688, 14), static_cast<uint32_t>(kSPV_KHR_abort)},
    {IR(3702, 17), static_cast<uint32_t>(kSPV_KHR_bfloat16)},
    {IR(3719, 25), static_cast<uint32_t>(kSPV_KHR_bit_instructions)},
    {IR(3744, 35), static_cast<uint32_t>(kSPV_KHR_compute_shader_derivatives)},
    {IR(3779, 22), static_cast<uint32_t>(kSPV_KHR_constant_data)},
    {IR(3801, 27), static_cast<uint32_t>(kSPV_KHR_cooperative_matrix)},
    {IR(3828, 21), static_cast<uint32_t>(kSPV_KHR_device_group)},
    {IR(3849, 22), static_cast<uint32_t>(kSPV_KHR_expect_assume)},
    {IR(3871, 23), static_cast<uint32_t>(kSPV_KHR_float_controls)},
    {IR(3894, 24), static_cast<uint32_t>(kSPV_KHR_float_controls2)},
    {IR(3918, 12), static_cast<uint32_t>(kSPV_KHR_fma)},
    {IR(3930, 36), static_cast<uint32_t>(kSPV_KHR_fragment_shader_barycentric)},
    {IR(3966, 30), static_cast<uint32_t>(kSPV_KHR_fragment_shading_rate)},
    {IR(3996, 28), static_cast<uint32_t>(kSPV_KHR_integer_dot_product)},
    {IR(4024, 21), static_cast<uint32_t>(kSPV_KHR_linkonce_odr)},
    {IR(4045, 30), static_cast<uint32_t>(kSPV_KHR_maximal_reconvergence)},
    {IR(4075, 18), static_cast<uint32_t>(kSPV_KHR_multiview)},
    {IR(4093, 35), static_cast<uint32_t>(kSPV_KHR_no_integer_wrap_decoration)},
    {IR(4128, 26), static_cast<uint32_t>(kSPV_KHR_non_semantic_info)},
    {IR(4154, 25), static_cast<uint32_t>(kSPV_KHR_opacity_micromap)},
    {IR(4179, 32), static_cast<uint32_t>(kSPV_KHR_physical_storage_buffer)},
    {IR(4211, 22), static_cast<uint32_t>(kSPV_KHR_poison_freeze)},
    {IR(4233, 28), static_cast<uint32_t>(kSPV_KHR_post_depth_coverage)},
    {IR(4261, 21), static_cast<uint32_t>(kSPV_KHR_quad_control)},
    {IR(4282, 22), static_cast<uint32_t>(kSPV_KHR_ray_cull_mask)},
    {IR(4304, 18), static_cast<uint32_t>(kSPV_KHR_ray_query)},
    {IR(4322, 20), static_cast<uint32_t>(kSPV_KHR_ray_tracing)},
    {IR(4342, 35), static_cast<uint32_t>(kSPV_KHR_ray_tracing_position_fetch)},
    {IR(4377, 37), static_cast<uint32_t>(kSPV_KHR_relaxed_extended_instruction)},
    {IR(4414, 34), static_cast<uint32_t>(kSPV_KHR_shader_atomic_counter_ops)},
    {IR(4448, 22), static_cast<uint32_t>(kSPV_KHR_shader_ballot)},
    {IR(4470, 21), static_cast<uint32_t>(kSPV_KHR_shader_clock)},
    {IR(4491, 31), static_cast<uint32_t>(kSPV_KHR_shader_draw_parameters)},
    {IR(4522, 37), static_cast<uint32_t>(kSPV_KHR_storage_buffer_storage_class)},
    {IR(4559, 24), static_cast<uint32_t>(kSPV_KHR_subgroup_rotate)},
    {IR(4583, 38), static_cast<uint32_t>(kSPV_KHR_subgroup_uniform_control_flow)},
    {IR(4621, 22), static_cast<uint32_t>(kSPV_KHR_subgroup_vote)},
    {IR(4643, 29), static_cast<uint32_t>(kSPV_KHR_terminate_invocation)},
    {IR(4672, 35), static_cast<uint32_t>(kSPV_KHR_uniform_group_instructions)},
    {IR(4707, 25), static_cast<uint32_t>(kSPV_KHR_untyped_pointers)},
    {IR(4732, 26), static_cast<uint32_t>(kSPV_KHR_variable_pointers)},
    {IR(4758, 28), static_cast<uint32_t>(kSPV_KHR_vulkan_memory_model)},
    {IR(4786, 41), static_cast<uint32_t>(kSPV_KHR_workgroup_memory_explicit_layout)},
    {IR(4827, 38), static_cast<uint32_t>(kSPV_NVX_multiview_per_view_attributes)},
    {IR(4865, 24), static_cast<uint32_t>(kSPV_NV_bindless_texture)},
    {IR(4889, 38), static_cast<uint32_t>(kSPV_NV_cluster_acceleration_structure)},
    {IR(4927, 34), static_cast<uint32_t>(kSPV_NV_compute_shader_derivatives)},
    {IR(4961, 26), static_cast<uint32_t>(kSPV_NV_cooperative_matrix)},
    {IR(4987, 27), static_cast<uint32_t>(kSPV_NV_cooperative_matrix2)},
    {IR(5014, 40), static_cast<uint32_t>(kSPV_NV_cooperative_matrix_decode_vector)},
    {IR(5054, 26), static_cast<uint32_t>(kSPV_NV_cooperative_vector)},
    {IR(5080, 29), static_cast<uint32_t>(kSPV_NV_displacement_micromap)},
    {IR(5109, 35), static_cast<uint32_t>(kSPV_NV_fragment_shader_barycentric)},
    {IR(5144, 35), static_cast<uint32_t>(kSPV_NV_geometry_shader_passthrough)},
    {IR(5179, 28), static_cast<uint32_t>(kSPV_NV_linear_swept_spheres)},
    {IR(5207, 19), static_cast<uint32_t>(kSPV_NV_mesh_shader)},
    {IR(5226, 26), static_cast<uint32_t>(kSPV_NV_push_constant_bank)},
    {IR(5252, 25), static_cast<uint32_t>(kSPV_NV_raw_access_chains)},
    {IR(5277, 19), static_cast<uint32_t>(kSPV_NV_ray_tracing)},
    {IR(5296, 31), static_cast<uint32_t>(kSPV_NV_ray_tracing_motion_blur)},
    {IR(5327, 37), static_cast<uint32_t>(kSPV_NV_sample_mask_override_coverage)},
    {IR(5364, 33), static_cast<uint32_t>(kSPV_NV_shader_atomic_fp16_vector)},
    {IR(5397, 30), static_cast<uint32_t>(kSPV_NV_shader_image_footprint)},
    {IR(5427, 33), static_cast<uint32_t>(kSPV_NV_shader_invocation_reorder)},
    {IR(5460, 26), static_cast<uint32_t>(kSPV_NV_shader_sm_builtins)},
    {IR(5486, 35), static_cast<uint32_t>(kSPV_NV_shader_subgroup_partitioned)},
    {IR(5521, 20), static_cast<uint32_t>(kSPV_NV_shading_rate)},
    {IR(5541, 29), static_cast<uint32_t>(kSPV_NV_stereo_view_rendering)},
    {IR(5570, 25), static_cast<uint32_t>(kSPV_NV_tensor_addressing)},
    {IR(5595, 23), static_cast<uint32_t>(kSPV_NV_viewport_array2)},
    {IR(5618, 25), static_cast<uint32_t>(kSPV_QCOM_bfloat16_muladd)},
    {IR(5643, 39), static_cast<uint32_t>(kSPV_QCOM_cooperative_matrix_conversion)},
    {IR(5682, 26), static_cast<uint32_t>(kSPV_QCOM_image_processing)},
    {IR(5708, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing2)},
    {IR(5735, 27), static_cast<uint32_t>(kSPV_QCOM_image_processing3)},
    {IR(5762, 30), static_cast<uint32_t>(kSPV_QCOM_multiple_wait_queues)},
    {IR(5792, 23), static_cast<uint32_t>(kSPV_QCOM_subgroup_size)},
    {IR(5815, 22), static_cast<uint32_t>(kSPV_QCOM_tile_shading)},
    {IR(5837, 22), static_cast<uint32_t>(kSPV_SAMSUNG_intrinsic)},
    {IR(5859, 38), static_cast<uint32_t>(kSPV_VALIDATOR_ignore_type_decl_unique)},
    {IR(5897, 34), static_cast<uint32_t>(kSPV_VALVE_mixed_float_dot_product)},
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
static const std::array<OperandDesc, 1251> kOperandsByValue{{
  {0,IR(0, 0),IR(5931, 9)/* ReadOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(5947, 10)/* WriteOnly */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(5957, 10)/* ReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(5967, 8)/* Logical */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {1,IR(0, 0),IR(5975, 11)/* Physical32 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {2,IR(0, 0),IR(5996, 11)/* Physical64 */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {5348,IR(0, 0),IR(6007, 24)/* PhysicalStorageBuffer64 */,IR(0, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {0,IR(0, 0),IR(6089, 9)/* Position */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {1,IR(0, 0),IR(6105, 10)/* PointSize */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {3,IR(0, 0),IR(6115, 13)/* ClipDistance */,IR(0, 0),IR(4, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4,IR(0, 0),IR(6128, 13)/* CullDistance */,IR(0, 0),IR(5, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5,IR(0, 0),IR(6141, 9)/* VertexId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6,IR(0, 0),IR(6150, 11)/* InstanceId */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {7,IR(0, 0),IR(6161, 12)/* PrimitiveId */,IR(0, 0),IR(6, 6),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {8,IR(0, 0),IR(6251, 13)/* InvocationId */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {9,IR(0, 0),IR(6264, 6)/* Layer */,IR(0, 0),IR(14, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {10,IR(0, 0),IR(6310, 14)/* ViewportIndex */,IR(0, 0),IR(19, 5),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {11,IR(0, 0),IR(6358, 15)/* TessLevelOuter */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {12,IR(0, 0),IR(6373, 15)/* TessLevelInner */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {13,IR(0, 0),IR(6388, 10)/* TessCoord */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {14,IR(0, 0),IR(6398, 14)/* PatchVertices */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {15,IR(0, 0),IR(6412, 10)/* FragCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {16,IR(0, 0),IR(6422, 11)/* PointCoord */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {17,IR(0, 0),IR(6433, 12)/* FrontFacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {18,IR(0, 0),IR(6445, 9)/* SampleId */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {19,IR(0, 0),IR(6472, 15)/* SamplePosition */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {20,IR(0, 0),IR(6487, 11)/* SampleMask */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {22,IR(0, 0),IR(6498, 10)/* FragDepth */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {23,IR(0, 0),IR(6508, 17)/* HelperInvocation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {24,IR(0, 0),IR(6525, 14)/* NumWorkgroups */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {25,IR(0, 0),IR(6539, 14)/* WorkgroupSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {26,IR(0, 0),IR(6553, 12)/* WorkgroupId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {27,IR(0, 0),IR(6565, 18)/* LocalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {28,IR(0, 0),IR(6583, 19)/* GlobalInvocationId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {29,IR(0, 0),IR(6602, 21)/* LocalInvocationIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {30,IR(0, 0),IR(6623, 8)/* WorkDim */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {31,IR(0, 0),IR(6631, 11)/* GlobalSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {32,IR(0, 0),IR(6642, 22)/* EnqueuedWorkgroupSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {33,IR(0, 0),IR(6664, 13)/* GlobalOffset */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {34,IR(0, 0),IR(6677, 15)/* GlobalLinearId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {36,IR(0, 0),IR(6692, 13)/* SubgroupSize */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {37,IR(0, 0),IR(6739, 16)/* SubgroupMaxSize */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {38,IR(0, 0),IR(6755, 13)/* NumSubgroups */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {39,IR(0, 0),IR(6768, 21)/* NumEnqueuedSubgroups */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {40,IR(0, 0),IR(6789, 11)/* SubgroupId */,IR(0, 0),IR(29, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {41,IR(0, 0),IR(6800, 26)/* SubgroupLocalInvocationId */,IR(0, 0),IR(26, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {42,IR(0, 0),IR(6826, 12)/* VertexIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {43,IR(0, 0),IR(6838, 14)/* InstanceIndex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4160,IR(0, 0),IR(6852, 10)/* CoreIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4161,IR(0, 0),IR(6878, 13)/* CoreCountARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4162,IR(0, 0),IR(6891, 13)/* CoreMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4163,IR(0, 0),IR(6904, 10)/* WarpIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4164,IR(0, 0),IR(6914, 13)/* WarpMaxIDARM */,IR(0, 0),IR(31, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4416,IR(0, 0),IR(6927, 15)/* SubgroupEqMask */,IR(1, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4417,IR(0, 0),IR(6982, 15)/* SubgroupGeMask */,IR(2, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4418,IR(0, 0),IR(7015, 15)/* SubgroupGtMask */,IR(3, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4419,IR(0, 0),IR(7048, 15)/* SubgroupLeMask */,IR(4, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4420,IR(0, 0),IR(7081, 15)/* SubgroupLtMask */,IR(5, 1),IR(32, 2),IR(2, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4424,IR(0, 0),IR(7114, 11)/* BaseVertex */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4425,IR(0, 0),IR(7140, 13)/* BaseInstance */,IR(0, 0),IR(34, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4426,IR(0, 0),IR(7153, 10)/* DrawIndex */,IR(0, 0),IR(35, 3),IR(4, 3),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4432,IR(0, 0),IR(7163, 24)/* PrimitiveShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4438,IR(0, 0),IR(7210, 12)/* DeviceIndex */,IR(0, 0),IR(39, 1),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4440,IR(0, 0),IR(7234, 10)/* ViewIndex */,IR(0, 0),IR(40, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4444,IR(0, 0),IR(7254, 15)/* ShadingRateKHR */,IR(0, 0),IR(38, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4492,IR(0, 0),IR(7269, 15)/* TileOffsetQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4493,IR(0, 0),IR(7300, 18)/* TileDimensionQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4494,IR(0, 0),IR(7318, 18)/* TileApronSizeQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4992,IR(0, 0),IR(7336, 20)/* BaryCoordNoPerspAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4993,IR(0, 0),IR(7356, 28)/* BaryCoordNoPerspCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4994,IR(0, 0),IR(7384, 26)/* BaryCoordNoPerspSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4995,IR(0, 0),IR(7410, 19)/* BaryCoordSmoothAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4996,IR(0, 0),IR(7429, 27)/* BaryCoordSmoothCentroidAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4997,IR(0, 0),IR(7456, 25)/* BaryCoordSmoothSampleAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {4998,IR(0, 0),IR(7481, 22)/* BaryCoordPullModelAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5014,IR(0, 0),IR(7503, 18)/* FragStencilRefEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5021,IR(0, 0),IR(7538, 29)/* RemainingRecursionLevelsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5073,IR(0, 0),IR(7585, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5122,IR(0, 0),IR(7601, 15)/* SamplerHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5123,IR(0, 0),IR(7634, 16)/* ResourceHeapEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5253,IR(0, 0),IR(7650, 15)/* ViewportMaskNV */,IR(0, 0),IR(45, 2),IR(12, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5257,IR(0, 0),IR(7686, 20)/* SecondaryPositionNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5258,IR(0, 0),IR(7725, 24)/* SecondaryViewportMaskNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5261,IR(0, 0),IR(7749, 18)/* PositionPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5262,IR(0, 0),IR(7787, 22)/* ViewportMaskPerViewNV */,IR(0, 0),IR(48, 2),IR(15, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5264,IR(0, 0),IR(7809, 16)/* FullyCoveredEXT */,IR(0, 0),IR(50, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5274,IR(0, 0),IR(7849, 12)/* TaskCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5275,IR(0, 0),IR(7861, 17)/* PrimitiveCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5276,IR(0, 0),IR(7878, 19)/* PrimitiveIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5277,IR(0, 0),IR(7897, 22)/* ClipDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5278,IR(0, 0),IR(7919, 22)/* CullDistancePerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5279,IR(0, 0),IR(7941, 15)/* LayerPerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5280,IR(0, 0),IR(7956, 16)/* MeshViewCountNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5281,IR(0, 0),IR(7972, 18)/* MeshViewIndicesNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5286,IR(0, 0),IR(7990, 13)/* BaryCoordKHR */,IR(6, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5287,IR(0, 0),IR(8038, 20)/* BaryCoordNoPerspKHR */,IR(7, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5292,IR(0, 0),IR(8077, 12)/* FragSizeEXT */,IR(8, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5293,IR(0, 0),IR(8123, 23)/* FragInvocationCountEXT */,IR(9, 1),IR(53, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5294,IR(0, 0),IR(8168, 25)/* PrimitivePointIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5295,IR(0, 0),IR(8193, 24)/* PrimitiveLineIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5296,IR(0, 0),IR(8217, 28)/* PrimitiveTriangleIndicesEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5299,IR(0, 0),IR(8245, 17)/* CullPrimitiveEXT */,IR(0, 0),IR(54, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5319,IR(0, 0),IR(8262, 12)/* LaunchIdKHR */,IR(10, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5320,IR(0, 0),IR(8285, 14)/* LaunchSizeKHR */,IR(11, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5321,IR(0, 0),IR(8312, 18)/* WorldRayOriginKHR */,IR(12, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5322,IR(0, 0),IR(8347, 21)/* WorldRayDirectionKHR */,IR(13, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5323,IR(0, 0),IR(8388, 19)/* ObjectRayOriginKHR */,IR(14, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5324,IR(0, 0),IR(8425, 22)/* ObjectRayDirectionKHR */,IR(15, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5325,IR(0, 0),IR(8468, 11)/* RayTminKHR */,IR(16, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5326,IR(0, 0),IR(8489, 11)/* RayTmaxKHR */,IR(17, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5327,IR(0, 0),IR(8510, 23)/* InstanceCustomIndexKHR */,IR(18, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5330,IR(0, 0),IR(8555, 17)/* ObjectToWorldKHR */,IR(19, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5331,IR(0, 0),IR(8588, 17)/* WorldToObjectKHR */,IR(20, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5332,IR(0, 0),IR(8621, 7)/* HitTNV */,IR(0, 0),IR(57, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5333,IR(0, 0),IR(8628, 11)/* HitKindKHR */,IR(21, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5334,IR(0, 0),IR(8649, 17)/* CurrentRayTimeNV */,IR(0, 0),IR(58, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5335,IR(0, 0),IR(8689, 30)/* HitTriangleVertexPositionsKHR */,IR(0, 0),IR(59, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5337,IR(0, 0),IR(8746, 34)/* HitMicroTriangleVertexPositionsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5344,IR(0, 0),IR(8813, 37)/* HitMicroTriangleVertexBarycentricsNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5351,IR(0, 0),IR(8850, 20)/* IncomingRayFlagsKHR */,IR(22, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5352,IR(0, 0),IR(8889, 20)/* RayGeometryIndexKHR */,IR(0, 0),IR(61, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5359,IR(0, 0),IR(8909, 14)/* HitIsSphereNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5360,IR(0, 0),IR(8951, 11)/* HitIsLSSNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5361,IR(0, 0),IR(9001, 20)/* HitSpherePositionNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5374,IR(0, 0),IR(9021, 13)/* WarpsPerSMNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5375,IR(0, 0),IR(9053, 10)/* SMCountNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5376,IR(0, 0),IR(9063, 9)/* WarpIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5377,IR(0, 0),IR(9072, 7)/* SMIDNV */,IR(0, 0),IR(64, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5396,IR(0, 0),IR(9079, 18)/* HitLSSPositionsNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5405,IR(0, 0),IR(9097, 34)/* HitKindFrontFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5406,IR(0, 0),IR(9131, 33)/* HitKindBackFacingMicroTriangleNV */,IR(0, 0),IR(60, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5420,IR(0, 0),IR(9164, 18)/* HitSphereRadiusNV */,IR(0, 0),IR(62, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5421,IR(0, 0),IR(9182, 14)/* HitLSSRadiiNV */,IR(0, 0),IR(63, 1),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {5436,IR(0, 0),IR(9196, 12)/* ClusterIDNV */,IR(0, 0),IR(65, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6021,IR(0, 0),IR(9249, 12)/* CullMaskKHR */,IR(0, 0),IR(66, 1),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {6186,IR(0, 0),IR(9276, 24)/* DeviceBarrierValidINTEL */,IR(0, 0),IR(67, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_BUILT_IN
  {0,IR(0, 0),IR(9319, 7)/* Matrix */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {1,IR(0, 0),IR(6098, 7)/* Shader */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {2,IR(0, 0),IR(6173, 9)/* Geometry */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {3,IR(0, 0),IR(6182, 13)/* Tessellation */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4,IR(0, 0),IR(5986, 10)/* Addresses */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5,IR(0, 0),IR(9326, 8)/* Linkage */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6,IR(0, 0),IR(5940, 7)/* Kernel */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7,IR(0, 0),IR(9334, 9)/* Vector16 */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {8,IR(0, 0),IR(9343, 14)/* Float16Buffer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {9,IR(0, 0),IR(9357, 8)/* Float16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {10,IR(0, 0),IR(9365, 8)/* Float64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {11,IR(0, 0),IR(9373, 6)/* Int64 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {12,IR(0, 0),IR(9379, 13)/* Int64Atomics */,IR(0, 0),IR(69, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {13,IR(0, 0),IR(9392, 11)/* ImageBasic */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {14,IR(0, 0),IR(9403, 15)/* ImageReadWrite */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {15,IR(0, 0),IR(9418, 12)/* ImageMipmap */,IR(0, 0),IR(70, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {17,IR(0, 0),IR(9430, 6)/* Pipes */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {18,IR(0, 0),IR(9436, 7)/* Groups */,IR(0, 0),IR(1, 0),IR(33, 1),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {19,IR(0, 0),IR(9443, 14)/* DeviceEnqueue */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {20,IR(0, 0),IR(9457, 15)/* LiteralSampler */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {21,IR(0, 0),IR(9472, 14)/* AtomicStorage */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {22,IR(0, 0),IR(9486, 6)/* Int16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {23,IR(0, 0),IR(9492, 22)/* TessellationPointSize */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {24,IR(0, 0),IR(9514, 18)/* GeometryPointSize */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {25,IR(0, 0),IR(9532, 20)/* ImageGatherExtended */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {27,IR(0, 0),IR(9552, 24)/* StorageImageMultisample */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {28,IR(0, 0),IR(9576, 34)/* UniformBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {29,IR(0, 0),IR(9610, 33)/* SampledImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {30,IR(0, 0),IR(9643, 34)/* StorageBufferArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {31,IR(0, 0),IR(9677, 33)/* StorageImageArrayDynamicIndexing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {32,IR(0, 0),IR(6115, 13)/* ClipDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {33,IR(0, 0),IR(6128, 13)/* CullDistance */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {34,IR(0, 0),IR(9710, 15)/* ImageCubeArray */,IR(0, 0),IR(72, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {35,IR(0, 0),IR(6454, 18)/* SampleRateShading */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {36,IR(0, 0),IR(9742, 10)/* ImageRect */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {37,IR(0, 0),IR(9752, 12)/* SampledRect */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {38,IR(0, 0),IR(9764, 15)/* GenericPointer */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {39,IR(0, 0),IR(9779, 5)/* Int8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {40,IR(0, 0),IR(9784, 16)/* InputAttachment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {41,IR(0, 0),IR(9800, 16)/* SparseResidency */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {42,IR(0, 0),IR(9816, 7)/* MinLod */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {43,IR(0, 0),IR(9823, 10)/* Sampled1D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {44,IR(0, 0),IR(9833, 8)/* Image1D */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {45,IR(0, 0),IR(9725, 17)/* SampledCubeArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {46,IR(0, 0),IR(9841, 14)/* SampledBuffer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {47,IR(0, 0),IR(9855, 12)/* ImageBuffer */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {48,IR(0, 0),IR(9867, 13)/* ImageMSArray */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {49,IR(0, 0),IR(9880, 28)/* StorageImageExtendedFormats */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {50,IR(0, 0),IR(9908, 11)/* ImageQuery */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {51,IR(0, 0),IR(9919, 18)/* DerivativeControl */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {52,IR(0, 0),IR(9937, 22)/* InterpolationFunction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {53,IR(0, 0),IR(9959, 18)/* TransformFeedback */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {54,IR(0, 0),IR(9977, 16)/* GeometryStreams */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {55,IR(0, 0),IR(9993, 30)/* StorageImageReadWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {56,IR(0, 0),IR(10023, 31)/* StorageImageWriteWithoutFormat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {57,IR(0, 0),IR(6324, 14)/* MultiViewport */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {58,IR(0, 0),IR(10054, 17)/* SubgroupDispatch */,IR(0, 0),IR(76, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {59,IR(0, 0),IR(10071, 13)/* NamedBarrier */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {60,IR(0, 0),IR(10084, 12)/* PipeStorage */,IR(0, 0),IR(77, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {61,IR(0, 0),IR(6705, 16)/* GroupNonUniform */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {62,IR(0, 0),IR(10096, 20)/* GroupNonUniformVote */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {63,IR(0, 0),IR(10116, 26)/* GroupNonUniformArithmetic */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {64,IR(0, 0),IR(6960, 22)/* GroupNonUniformBallot */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {65,IR(0, 0),IR(10142, 23)/* GroupNonUniformShuffle */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {66,IR(0, 0),IR(10165, 31)/* GroupNonUniformShuffleRelative */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {67,IR(0, 0),IR(10196, 25)/* GroupNonUniformClustered */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {68,IR(0, 0),IR(10221, 20)/* GroupNonUniformQuad */,IR(0, 0),IR(78, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {69,IR(0, 0),IR(6270, 12)/* ShaderLayer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {70,IR(0, 0),IR(6338, 20)/* ShaderViewportIndex */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {71,IR(0, 0),IR(10241, 18)/* UniformDecoration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4165,IR(0, 0),IR(6862, 16)/* CoreBuiltinsARM */,IR(0, 0),IR(1, 0),IR(34, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4166,IR(0, 0),IR(10259, 28)/* TileImageColorReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4167,IR(0, 0),IR(10287, 28)/* TileImageDepthReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4168,IR(0, 0),IR(10315, 30)/* TileImageStencilReadAccessEXT */,IR(0, 0),IR(1, 0),IR(35, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4174,IR(0, 0),IR(10345, 11)/* TensorsARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4175,IR(0, 0),IR(10356, 37)/* StorageTensorArrayDynamicIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4176,IR(0, 0),IR(10393, 40)/* StorageTensorArrayNonUniformIndexingARM */,IR(0, 0),IR(1, 0),IR(36, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4191,IR(0, 0),IR(10433, 9)/* GraphARM */,IR(0, 0),IR(1, 0),IR(37, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4201,IR(0, 0),IR(10442, 28)/* CooperativeMatrixLayoutsARM */,IR(0, 0),IR(1, 0),IR(38, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4212,IR(0, 0),IR(10470, 10)/* Float8EXT */,IR(0, 0),IR(1, 0),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4213,IR(0, 0),IR(10480, 27)/* Float8CooperativeMatrixEXT */,IR(0, 0),IR(79, 2),IR(39, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4228,IR(0, 0),IR(10528, 10)/* Float6EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4229,IR(0, 0),IR(10538, 10)/* Float4EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4230,IR(0, 0),IR(10548, 22)/* Float8UnsignedE8M0EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4231,IR(0, 0),IR(10570, 10)/* MXInt8EXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4232,IR(0, 0),IR(10580, 18)/* BitcastExtractEXT */,IR(0, 0),IR(1, 0),IR(40, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4422,IR(0, 0),IR(7187, 23)/* FragmentShadingRateKHR */,IR(0, 0),IR(3, 1),IR(7, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4423,IR(0, 0),IR(6721, 18)/* SubgroupBallotKHR */,IR(0, 0),IR(1, 0),IR(2, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4427,IR(0, 0),IR(7125, 15)/* DrawParameters */,IR(0, 0),IR(3, 1),IR(3, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4428,IR(0, 0),IR(10598, 33)/* WorkgroupMemoryExplicitLayoutKHR */,IR(0, 0),IR(3, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4429,IR(0, 0),IR(10631, 43)/* WorkgroupMemoryExplicitLayout8BitAccessKHR */,IR(0, 0),IR(81, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4430,IR(0, 0),IR(10674, 44)/* WorkgroupMemoryExplicitLayout16BitAccessKHR */,IR(0, 0),IR(81, 1),IR(41, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4431,IR(0, 0),IR(10718, 16)/* SubgroupVoteKHR */,IR(0, 0),IR(1, 0),IR(42, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4433,IR(0, 0),IR(10734, 25)/* StorageBuffer16BitAccess */,IR(23, 1),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4434,IR(0, 0),IR(10787, 35)/* UniformAndStorageBuffer16BitAccess */,IR(24, 1),IR(82, 1),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4435,IR(0, 0),IR(10839, 22)/* StoragePushConstant16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4436,IR(0, 0),IR(10861, 21)/* StorageInputOutput16 */,IR(0, 0),IR(1, 0),IR(43, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4437,IR(0, 0),IR(7222, 12)/* DeviceGroup */,IR(0, 0),IR(1, 0),IR(8, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4439,IR(0, 0),IR(7244, 10)/* MultiView */,IR(0, 0),IR(3, 1),IR(9, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4441,IR(0, 0),IR(10882, 30)/* VariablePointersStorageBuffer */,IR(0, 0),IR(3, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4442,IR(0, 0),IR(10912, 17)/* VariablePointers */,IR(0, 0),IR(83, 1),IR(44, 1),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4445,IR(0, 0),IR(10929, 17)/* AtomicStorageOps */,IR(0, 0),IR(84, 1),IR(45, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4447,IR(0, 0),IR(10946, 28)/* SampleMaskPostDepthCoverage */,IR(0, 0),IR(1, 0),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4448,IR(0, 0),IR(10974, 24)/* StorageBuffer8BitAccess */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4449,IR(0, 0),IR(10998, 34)/* UniformAndStorageBuffer8BitAccess */,IR(0, 0),IR(85, 1),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4450,IR(0, 0),IR(11032, 21)/* StoragePushConstant8 */,IR(0, 0),IR(1, 0),IR(47, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4464,IR(0, 0),IR(11053, 15)/* DenormPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4465,IR(0, 0),IR(11068, 18)/* DenormFlushToZero */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4466,IR(0, 0),IR(11086, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4467,IR(0, 0),IR(11111, 16)/* RoundingModeRTE */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4468,IR(0, 0),IR(11127, 16)/* RoundingModeRTZ */,IR(0, 0),IR(1, 0),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4471,IR(0, 0),IR(11143, 23)/* RayQueryProvisionalKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4472,IR(0, 0),IR(11166, 12)/* RayQueryKHR */,IR(0, 0),IR(3, 1),IR(49, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4473,IR(0, 0),IR(11178, 19)/* UntypedPointersKHR */,IR(0, 0),IR(1, 0),IR(50, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4478,IR(0, 0),IR(11197, 32)/* RayTraversalPrimitiveCullingKHR */,IR(0, 0),IR(86, 2),IR(51, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4479,IR(0, 0),IR(6208, 14)/* RayTracingKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4484,IR(0, 0),IR(11229, 26)/* TextureSampleWeightedQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4485,IR(0, 0),IR(11255, 21)/* TextureBoxFilterQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4486,IR(0, 0),IR(11276, 22)/* TextureBlockMatchQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4495,IR(0, 0),IR(7284, 16)/* TileShadingQCOM */,IR(0, 0),IR(3, 1),IR(54, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4496,IR(0, 0),IR(11298, 32)/* CooperativeMatrixConversionQCOM */,IR(0, 0),IR(88, 1),IR(55, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4498,IR(0, 0),IR(11330, 23)/* TextureBlockMatch2QCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4504,IR(0, 0),IR(11353, 19)/* BFloat16MulAddQCOM */,IR(0, 0),IR(1, 0),IR(57, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4506,IR(0, 0),IR(11372, 17)/* SubgroupSizeQCOM */,IR(0, 0),IR(1, 0),IR(58, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4539,IR(0, 0),IR(11389, 23)/* MultipleWaitQueuesQCOM */,IR(0, 0),IR(1, 0),IR(59, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4543,IR(0, 0),IR(11412, 22)/* ImageGatherLinearQCOM */,IR(0, 0),IR(1, 0),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {4544,IR(0, 0),IR(11434, 29)/* ImageGatherExtendedModesQCOM */,IR(0, 0),IR(1, 0),IR(60, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5008,IR(0, 0),IR(11463, 16)/* Float16ImageAMD */,IR(0, 0),IR(3, 1),IR(61, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5009,IR(0, 0),IR(11479, 22)/* ImageGatherBiasLodAMD */,IR(0, 0),IR(3, 1),IR(62, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5010,IR(0, 0),IR(11501, 16)/* FragmentMaskAMD */,IR(0, 0),IR(3, 1),IR(63, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5013,IR(0, 0),IR(7521, 17)/* StencilExportEXT */,IR(0, 0),IR(3, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5015,IR(0, 0),IR(11517, 21)/* ImageReadWriteLodAMD */,IR(0, 0),IR(3, 1),IR(64, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5016,IR(0, 0),IR(11538, 14)/* Int64ImageEXT */,IR(0, 0),IR(69, 1),IR(65, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5055,IR(0, 0),IR(11552, 15)/* ShaderClockKHR */,IR(0, 0),IR(1, 0),IR(66, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5067,IR(0, 0),IR(7567, 18)/* ShaderEnqueueAMDX */,IR(0, 0),IR(3, 1),IR(67, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5087,IR(0, 0),IR(11567, 15)/* QuadControlKHR */,IR(0, 0),IR(1, 0),IR(68, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5112,IR(0, 0),IR(11582, 14)/* Int4TypeINTEL */,IR(0, 0),IR(1, 0),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5114,IR(0, 0),IR(11596, 27)/* Int4CooperativeMatrixINTEL */,IR(0, 0),IR(89, 2),IR(69, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5116,IR(0, 0),IR(11623, 16)/* BFloat16TypeKHR */,IR(0, 0),IR(1, 0),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5117,IR(0, 0),IR(11639, 22)/* BFloat16DotProductKHR */,IR(0, 0),IR(91, 1),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5118,IR(0, 0),IR(11661, 29)/* BFloat16CooperativeMatrixKHR */,IR(0, 0),IR(92, 2),IR(70, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5120,IR(0, 0),IR(11690, 9)/* AbortKHR */,IR(0, 0),IR(1, 0),IR(71, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5128,IR(0, 0),IR(7616, 18)/* DescriptorHeapEXT */,IR(0, 0),IR(94, 1),IR(72, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5146,IR(0, 0),IR(11699, 16)/* ConstantDataKHR */,IR(0, 0),IR(1, 0),IR(73, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5156,IR(0, 0),IR(11715, 16)/* PoisonFreezeKHR */,IR(0, 0),IR(1, 0),IR(74, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5181,IR(0, 0),IR(11731, 15)/* WeakLinkageAMD */,IR(0, 0),IR(95, 1),IR(75, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5249,IR(0, 0),IR(11746, 29)/* SampleMaskOverrideCoverageNV */,IR(0, 0),IR(25, 1),IR(76, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5251,IR(0, 0),IR(11775, 28)/* GeometryShaderPassthroughNV */,IR(0, 0),IR(71, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5254,IR(0, 0),IR(6282, 28)/* ShaderViewportIndexLayerEXT */,IR(25, 1),IR(96, 1),IR(78, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5255,IR(0, 0),IR(7665, 21)/* ShaderViewportMaskNV */,IR(0, 0),IR(97, 1),IR(80, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5259,IR(0, 0),IR(7706, 19)/* ShaderStereoViewNV */,IR(0, 0),IR(98, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5260,IR(0, 0),IR(7767, 20)/* PerViewAttributesNV */,IR(0, 0),IR(40, 1),IR(81, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5265,IR(0, 0),IR(7825, 24)/* FragmentFullyCoveredEXT */,IR(0, 0),IR(3, 1),IR(17, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5266,IR(0, 0),IR(6222, 14)/* MeshShadingNV */,IR(0, 0),IR(3, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5282,IR(0, 0),IR(11830, 17)/* ImageFootprintNV */,IR(0, 0),IR(1, 0),IR(82, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5283,IR(0, 0),IR(6236, 15)/* MeshShadingEXT */,IR(0, 0),IR(3, 1),IR(23, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5284,IR(0, 0),IR(8015, 23)/* FragmentBarycentricKHR */,IR(26, 1),IR(1, 0),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5288,IR(0, 0),IR(11869, 31)/* ComputeDerivativeGroupQuadsKHR */,IR(27, 1),IR(3, 1),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5291,IR(0, 0),IR(8104, 19)/* FragmentDensityEXT */,IR(28, 1),IR(3, 1),IR(21, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5297,IR(0, 0),IR(11944, 30)/* GroupNonUniformPartitionedEXT */,IR(29, 1),IR(1, 0),IR(85, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5301,IR(0, 0),IR(12003, 17)/* ShaderNonUniform */,IR(30, 1),IR(3, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5302,IR(0, 0),IR(12040, 23)/* RuntimeDescriptorArray */,IR(31, 1),IR(3, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5303,IR(0, 0),IR(12089, 36)/* InputAttachmentArrayDynamicIndexing */,IR(32, 1),IR(99, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5304,IR(0, 0),IR(12164, 39)/* UniformTexelBufferArrayDynamicIndexing */,IR(33, 1),IR(75, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5305,IR(0, 0),IR(12245, 39)/* StorageTexelBufferArrayDynamicIndexing */,IR(34, 1),IR(100, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5306,IR(0, 0),IR(12326, 37)/* UniformBufferArrayNonUniformIndexing */,IR(35, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5307,IR(0, 0),IR(12403, 36)/* SampledImageArrayNonUniformIndexing */,IR(36, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5308,IR(0, 0),IR(12478, 37)/* StorageBufferArrayNonUniformIndexing */,IR(37, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5309,IR(0, 0),IR(12555, 36)/* StorageImageArrayNonUniformIndexing */,IR(38, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5310,IR(0, 0),IR(12630, 39)/* InputAttachmentArrayNonUniformIndexing */,IR(39, 1),IR(102, 2),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5311,IR(0, 0),IR(12711, 42)/* UniformTexelBufferArrayNonUniformIndexing */,IR(40, 1),IR(104, 2),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5312,IR(0, 0),IR(12798, 42)/* StorageTexelBufferArrayNonUniformIndexing */,IR(41, 1),IR(106, 2),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5336,IR(0, 0),IR(8719, 27)/* RayTracingPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5340,IR(0, 0),IR(6195, 13)/* RayTracingNV */,IR(0, 0),IR(3, 1),IR(26, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5341,IR(0, 0),IR(8666, 23)/* RayTracingMotionBlurNV */,IR(0, 0),IR(3, 1),IR(27, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5345,IR(0, 0),IR(12885, 18)/* VulkanMemoryModel */,IR(42, 1),IR(1, 0),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5346,IR(0, 0),IR(12924, 29)/* VulkanMemoryModelDeviceScope */,IR(43, 1),IR(1, 0),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5347,IR(0, 0),IR(6058, 31)/* PhysicalStorageBufferAddresses */,IR(44, 1),IR(3, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5350,IR(0, 0),IR(13019, 32)/* ComputeDerivativeGroupLinearKHR */,IR(45, 1),IR(3, 1),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5353,IR(0, 0),IR(13082, 25)/* RayTracingProvisionalKHR */,IR(0, 0),IR(3, 1),IR(28, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5357,IR(0, 0),IR(13107, 20)/* CooperativeMatrixNV */,IR(0, 0),IR(3, 1),IR(90, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5363,IR(0, 0),IR(13127, 33)/* FragmentShaderSampleInterlockEXT */,IR(0, 0),IR(3, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5372,IR(0, 0),IR(13160, 38)/* FragmentShaderShadingRateInterlockEXT */,IR(0, 0),IR(3, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5373,IR(0, 0),IR(9034, 19)/* ShaderSMBuiltinsNV */,IR(0, 0),IR(3, 1),IR(30, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5378,IR(0, 0),IR(13198, 32)/* FragmentShaderPixelInterlockEXT */,IR(0, 0),IR(3, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5379,IR(0, 0),IR(13230, 25)/* DemoteToHelperInvocation */,IR(46, 1),IR(3, 1),IR(92, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5380,IR(0, 0),IR(13283, 23)/* DisplacementMicromapNV */,IR(0, 0),IR(3, 1),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5381,IR(0, 0),IR(13306, 29)/* RayTracingOpacityMicromapKHR */,IR(47, 1),IR(3, 1),IR(94, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5383,IR(0, 0),IR(13364, 26)/* ShaderInvocationReorderNV */,IR(0, 0),IR(61, 1),IR(96, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5388,IR(0, 0),IR(13390, 27)/* ShaderInvocationReorderEXT */,IR(0, 0),IR(61, 1),IR(97, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5390,IR(0, 0),IR(13417, 18)/* BindlessTextureNV */,IR(0, 0),IR(1, 0),IR(98, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5391,IR(0, 0),IR(13435, 25)/* RayQueryPositionFetchKHR */,IR(0, 0),IR(3, 1),IR(88, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5394,IR(0, 0),IR(13460, 20)/* CooperativeVectorNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5404,IR(0, 0),IR(13480, 22)/* AtomicFloat16VectorNV */,IR(0, 0),IR(1, 0),IR(100, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5409,IR(0, 0),IR(8780, 33)/* RayTracingDisplacementMicromapNV */,IR(0, 0),IR(61, 1),IR(93, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5414,IR(0, 0),IR(13502, 18)/* RawAccessChainsNV */,IR(0, 0),IR(1, 0),IR(101, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5418,IR(0, 0),IR(8923, 28)/* RayTracingSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5419,IR(0, 0),IR(8962, 39)/* RayTracingLinearSweptSpheresGeometryNV */,IR(0, 0),IR(1, 0),IR(29, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5423,IR(0, 0),IR(13520, 20)/* PushConstantBanksNV */,IR(0, 0),IR(3, 1),IR(102, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5425,IR(0, 0),IR(13540, 14)/* LongVectorEXT */,IR(0, 0),IR(1, 0),IR(103, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5426,IR(0, 0),IR(13554, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(1, 0),IR(104, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5429,IR(0, 0),IR(13577, 32)/* CooperativeMatrixConversionsEXT */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5430,IR(0, 0),IR(13609, 31)/* CooperativeMatrixReductionsEXT */,IR(48, 1),IR(1, 0),IR(106, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5431,IR(0, 0),IR(13670, 31)/* CooperativeMatrixConversionsNV */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5432,IR(0, 0),IR(13701, 41)/* CooperativeMatrixPerElementOperationsEXT */,IR(49, 1),IR(1, 0),IR(106, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5433,IR(0, 0),IR(13782, 36)/* CooperativeMatrixTensorAddressingNV */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5434,IR(0, 0),IR(13818, 30)/* CooperativeMatrixBlockLoadsNV */,IR(0, 0),IR(1, 0),IR(108, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5435,IR(0, 0),IR(13848, 28)/* CooperativeVectorTrainingNV */,IR(0, 0),IR(1, 0),IR(99, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5437,IR(0, 0),IR(9208, 41)/* RayTracingClusterAccelerationStructureNV */,IR(0, 0),IR(61, 1),IR(31, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5438,IR(0, 0),IR(13876, 34)/* CooperativeMatrixGetCoordinateEXT */,IR(0, 0),IR(1, 0),IR(105, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5439,IR(0, 0),IR(13910, 19)/* TensorAddressingNV */,IR(0, 0),IR(1, 0),IR(109, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5447,IR(0, 0),IR(13929, 32)/* CooperativeMatrixDecodeVectorNV */,IR(0, 0),IR(108, 1),IR(110, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5568,IR(0, 0),IR(13961, 21)/* SubgroupShuffleINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5569,IR(0, 0),IR(13982, 27)/* SubgroupBufferBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5570,IR(0, 0),IR(14009, 26)/* SubgroupImageBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(111, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5579,IR(0, 0),IR(14035, 31)/* SubgroupImageMediaBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(112, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5582,IR(0, 0),IR(14066, 21)/* RoundToInfinityINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5583,IR(0, 0),IR(14087, 23)/* FloatingPointModeINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5584,IR(0, 0),IR(14110, 23)/* IntegerFunctions2INTEL */,IR(0, 0),IR(1, 0),IR(114, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5603,IR(0, 0),IR(14133, 22)/* FunctionPointersINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5604,IR(0, 0),IR(14155, 24)/* IndirectReferencesINTEL */,IR(0, 0),IR(1, 0),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5606,IR(0, 0),IR(14179, 9)/* AsmINTEL */,IR(0, 0),IR(1, 0),IR(116, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5612,IR(0, 0),IR(14188, 23)/* AtomicFloat32MinMaxEXT */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5613,IR(0, 0),IR(14211, 23)/* AtomicFloat64MinMaxEXT */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5616,IR(0, 0),IR(14234, 23)/* AtomicFloat16MinMaxEXT */,IR(0, 0),IR(1, 0),IR(117, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5617,IR(0, 0),IR(14257, 19)/* VectorComputeINTEL */,IR(0, 0),IR(109, 1),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5619,IR(0, 0),IR(14276, 15)/* VectorAnyINTEL */,IR(0, 0),IR(1, 0),IR(118, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5629,IR(0, 0),IR(14291, 16)/* ExpectAssumeKHR */,IR(0, 0),IR(1, 0),IR(119, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5696,IR(0, 0),IR(14307, 33)/* SubgroupAvcMotionEstimationINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5697,IR(0, 0),IR(14340, 38)/* SubgroupAvcMotionEstimationIntraINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5698,IR(0, 0),IR(14378, 39)/* SubgroupAvcMotionEstimationChromaINTEL */,IR(0, 0),IR(1, 0),IR(120, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5817,IR(0, 0),IR(14417, 25)/* VariableLengthArrayINTEL */,IR(0, 0),IR(1, 0),IR(121, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5821,IR(0, 0),IR(14442, 26)/* FunctionFloatControlINTEL */,IR(0, 0),IR(1, 0),IR(113, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5824,IR(0, 0),IR(14468, 27)/* FPGAMemoryAttributesALTERA */,IR(50, 1),IR(1, 0),IR(122, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5837,IR(0, 0),IR(14521, 20)/* FPFastMathModeINTEL */,IR(0, 0),IR(0, 1),IR(124, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5844,IR(0, 0),IR(14541, 33)/* ArbitraryPrecisionIntegersALTERA */,IR(51, 1),IR(1, 0),IR(125, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5845,IR(0, 0),IR(14606, 38)/* ArbitraryPrecisionFloatingPointALTERA */,IR(52, 1),IR(1, 0),IR(127, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5886,IR(0, 0),IR(14681, 30)/* UnstructuredLoopControlsINTEL */,IR(0, 0),IR(1, 0),IR(129, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5888,IR(0, 0),IR(14711, 23)/* FPGALoopControlsALTERA */,IR(53, 1),IR(1, 0),IR(130, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5892,IR(0, 0),IR(14756, 22)/* KernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5897,IR(0, 0),IR(14778, 26)/* FPGAKernelAttributesINTEL */,IR(0, 0),IR(1, 0),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5898,IR(0, 0),IR(14804, 25)/* FPGAMemoryAccessesALTERA */,IR(54, 1),IR(1, 0),IR(133, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5904,IR(0, 0),IR(14853, 28)/* FPGAClusterAttributesALTERA */,IR(55, 1),IR(1, 0),IR(135, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5906,IR(0, 0),IR(14908, 15)/* LoopFuseALTERA */,IR(56, 1),IR(1, 0),IR(137, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5908,IR(0, 0),IR(14937, 21)/* FPGADSPControlALTERA */,IR(57, 1),IR(1, 0),IR(139, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5910,IR(0, 0),IR(14978, 26)/* MemoryAccessAliasingINTEL */,IR(0, 0),IR(1, 0),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5916,IR(0, 0),IR(15004, 41)/* FPGAInvocationPipeliningAttributesALTERA */,IR(58, 1),IR(1, 0),IR(142, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5920,IR(0, 0),IR(15085, 25)/* FPGABufferLocationALTERA */,IR(59, 1),IR(1, 0),IR(144, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5922,IR(0, 0),IR(15134, 35)/* ArbitraryPrecisionFixedPointALTERA */,IR(60, 1),IR(1, 0),IR(146, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5935,IR(0, 0),IR(15203, 24)/* USMStorageClassesALTERA */,IR(61, 1),IR(1, 0),IR(148, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5939,IR(0, 0),IR(15250, 30)/* RuntimeAlignedAttributeALTERA */,IR(62, 1),IR(1, 0),IR(150, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5943,IR(0, 0),IR(15309, 14)/* IOPipesALTERA */,IR(63, 1),IR(1, 0),IR(152, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5945,IR(0, 0),IR(15336, 20)/* BlockingPipesALTERA */,IR(64, 1),IR(1, 0),IR(154, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {5948,IR(0, 0),IR(15375, 14)/* FPGARegALTERA */,IR(65, 1),IR(1, 0),IR(156, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6016,IR(0, 0),IR(15402, 19)/* DotProductInputAll */,IR(66, 1),IR(1, 0),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6017,IR(0, 0),IR(15443, 22)/* DotProductInput4x8Bit */,IR(67, 1),IR(110, 1),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6018,IR(0, 0),IR(15490, 28)/* DotProductInput4x8BitPacked */,IR(68, 1),IR(1, 0),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6019,IR(0, 0),IR(15549, 11)/* DotProduct */,IR(69, 1),IR(1, 0),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6020,IR(0, 0),IR(9261, 15)/* RayCullMaskKHR */,IR(0, 0),IR(1, 0),IR(32, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6022,IR(0, 0),IR(10507, 21)/* CooperativeMatrixKHR */,IR(0, 0),IR(1, 0),IR(159, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6024,IR(0, 0),IR(15574, 24)/* ReplicatedCompositesEXT */,IR(0, 0),IR(1, 0),IR(160, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6025,IR(0, 0),IR(15598, 16)/* BitInstructions */,IR(0, 0),IR(1, 0),IR(161, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6026,IR(0, 0),IR(15614, 25)/* GroupNonUniformRotateKHR */,IR(0, 0),IR(78, 1),IR(162, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6029,IR(0, 0),IR(15639, 15)/* FloatControls2 */,IR(0, 0),IR(1, 0),IR(163, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6030,IR(0, 0),IR(15654, 7)/* FMAKHR */,IR(0, 0),IR(1, 0),IR(164, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6032,IR(0, 0),IR(15661, 42)/* RayTracingOpacityMicromapExecutionModeKHR */,IR(0, 0),IR(3, 1),IR(165, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6033,IR(0, 0),IR(15703, 20)/* AtomicFloat32AddEXT */,IR(0, 0),IR(1, 0),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6034,IR(0, 0),IR(15723, 20)/* AtomicFloat64AddEXT */,IR(0, 0),IR(1, 0),IR(166, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6089,IR(0, 0),IR(15743, 20)/* LongCompositesINTEL */,IR(0, 0),IR(1, 0),IR(167, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6094,IR(0, 0),IR(15763, 11)/* OptNoneEXT */,IR(70, 1),IR(1, 0),IR(168, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6095,IR(0, 0),IR(15787, 20)/* AtomicFloat16AddEXT */,IR(0, 0),IR(1, 0),IR(170, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6114,IR(0, 0),IR(15807, 21)/* DebugInfoModuleINTEL */,IR(0, 0),IR(1, 0),IR(171, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6115,IR(0, 0),IR(15828, 24)/* BFloat16ConversionINTEL */,IR(0, 0),IR(1, 0),IR(172, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6141,IR(0, 0),IR(15852, 16)/* SplitBarrierEXT */,IR(71, 1),IR(1, 0),IR(173, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6144,IR(0, 0),IR(15886, 19)/* ArithmeticFenceEXT */,IR(0, 0),IR(1, 0),IR(175, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6150,IR(0, 0),IR(15905, 30)/* FPGAClusterAttributesV2ALTERA */,IR(72, 1),IR(111, 1),IR(135, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6161,IR(0, 0),IR(15964, 28)/* FPGAKernelAttributesv2INTEL */,IR(0, 0),IR(112, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6162,IR(0, 0),IR(15992, 19)/* TaskSequenceALTERA */,IR(73, 1),IR(1, 0),IR(176, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6169,IR(0, 0),IR(16029, 16)/* FPMaxErrorINTEL */,IR(0, 0),IR(1, 0),IR(178, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6171,IR(0, 0),IR(16045, 25)/* FPGALatencyControlALTERA */,IR(74, 1),IR(1, 0),IR(179, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6174,IR(0, 0),IR(16094, 29)/* FPGAArgumentInterfacesALTERA */,IR(75, 1),IR(1, 0),IR(181, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6185,IR(0, 0),IR(9300, 19)/* DeviceBarrierINTEL */,IR(0, 0),IR(1, 0),IR(183, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6187,IR(0, 0),IR(16151, 30)/* GlobalVariableHostAccessINTEL */,IR(0, 0),IR(1, 0),IR(184, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6189,IR(0, 0),IR(16181, 36)/* GlobalVariableFPGADecorationsALTERA */,IR(76, 1),IR(1, 0),IR(185, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6220,IR(0, 0),IR(16252, 28)/* SubgroupBufferPrefetchINTEL */,IR(0, 0),IR(1, 0),IR(187, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6228,IR(0, 0),IR(16280, 23)/* Subgroup2DBlockIOINTEL */,IR(0, 0),IR(1, 0),IR(188, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6229,IR(0, 0),IR(16303, 30)/* Subgroup2DBlockTransformINTEL */,IR(0, 0),IR(113, 1),IR(188, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6230,IR(0, 0),IR(16333, 30)/* Subgroup2DBlockTransposeINTEL */,IR(0, 0),IR(113, 1),IR(188, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6236,IR(0, 0),IR(16363, 38)/* SubgroupMatrixMultiplyAccumulateINTEL */,IR(0, 0),IR(1, 0),IR(189, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6241,IR(0, 0),IR(16401, 28)/* TernaryBitwiseFunctionINTEL */,IR(0, 0),IR(1, 0),IR(190, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6243,IR(0, 0),IR(16429, 32)/* UntypedVariableLengthArrayINTEL */,IR(0, 0),IR(114, 2),IR(121, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6245,IR(0, 0),IR(16461, 21)/* SpecConditionalINTEL */,IR(0, 0),IR(1, 0),IR(191, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6246,IR(0, 0),IR(16482, 22)/* FunctionVariantsINTEL */,IR(0, 0),IR(116, 1),IR(191, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6257,IR(0, 0),IR(16504, 18)/* PredicatedIOINTEL */,IR(0, 0),IR(1, 0),IR(192, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6265,IR(0, 0),IR(16522, 23)/* RoundedDivideSqrtINTEL */,IR(0, 0),IR(1, 0),IR(193, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6400,IR(0, 0),IR(16545, 26)/* GroupUniformArithmeticKHR */,IR(0, 0),IR(1, 0),IR(194, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6425,IR(0, 0),IR(16571, 27)/* TensorFloat32RoundingINTEL */,IR(0, 0),IR(1, 0),IR(195, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6427,IR(0, 0),IR(16598, 25)/* MaskedGatherScatterINTEL */,IR(0, 0),IR(1, 0),IR(196, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6441,IR(0, 0),IR(16623, 19)/* CacheControlsINTEL */,IR(0, 0),IR(1, 0),IR(197, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6460,IR(0, 0),IR(16642, 20)/* RegisterLimitsINTEL */,IR(0, 0),IR(1, 0),IR(198, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6528,IR(0, 0),IR(16662, 20)/* BindlessImagesINTEL */,IR(0, 0),IR(1, 0),IR(199, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6912,IR(0, 0),IR(16682, 33)/* DotProductFloat16AccFloat32VALVE */,IR(0, 0),IR(117, 1),IR(200, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6913,IR(0, 0),IR(16715, 33)/* DotProductFloat16AccFloat16VALVE */,IR(0, 0),IR(117, 1),IR(200, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6914,IR(0, 0),IR(16748, 27)/* DotProductBFloat16AccVALVE */,IR(0, 0),IR(91, 1),IR(200, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {6915,IR(0, 0),IR(16775, 32)/* DotProductFloat8AccFloat32VALVE */,IR(0, 0),IR(118, 1),IR(200, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {7041,IR(0, 0),IR(16807, 17)/* IntrinsicSAMSUNG */,IR(0, 0),IR(1, 0),IR(201, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_CAPABILITY
  {0,IR(0, 0),IR(16824, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16836, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16844, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16852, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16858, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16865, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16876, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16885, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16898, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16904, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16914, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16920, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16935, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16960, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16976, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16990, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(17003, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(17015, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(17032, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(17044, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(17059, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(17072, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(17087, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(17105, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(17122, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(17143, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17163, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17183, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(17199, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(17215, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(17235, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17259, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(17265, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(17270, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {3,IR(0, 1),IR(17309, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {4,IR(1, 2),IR(17320, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(17329, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(17334, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(17341, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {8,IR(0, 1),IR(17352, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {9,IR(1, 2),IR(17359, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(17368, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17378, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17391, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(17404, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(17415, 10)/* Float16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1,IR(0, 0),IR(17425, 10)/* Float32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {2,IR(0, 0),IR(17435, 10)/* Float64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {3,IR(0, 0),IR(17445, 13)/* SignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {4,IR(0, 0),IR(17458, 14)/* SignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {5,IR(0, 0),IR(17472, 14)/* SignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {6,IR(0, 0),IR(17486, 14)/* SignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {7,IR(0, 0),IR(17500, 15)/* UnsignedInt8NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {8,IR(0, 0),IR(17515, 16)/* UnsignedInt16NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {9,IR(0, 0),IR(17531, 16)/* UnsignedInt32NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {10,IR(0, 0),IR(17547, 16)/* UnsignedInt64NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491000,IR(0, 0),IR(17563, 19)/* SignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491001,IR(0, 0),IR(17582, 21)/* UnsignedInt8PackedNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491002,IR(0, 0),IR(17603, 12)/* FloatE4M3NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {1000491003,IR(0, 0),IR(17615, 12)/* FloatE5M2NV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COMPONENT_TYPE
  {0,IR(0, 0),IR(17627, 12)/* RowMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17639, 15)/* ColumnMajorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4202,IR(0, 0),IR(17654, 25)/* RowBlockedInterleavedARM */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {4203,IR(0, 0),IR(17679, 28)/* ColumnBlockedInterleavedARM */,IR(0, 0),IR(119, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {0,IR(0, 0),IR(17707, 8)/* NoneKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17715, 27)/* MatrixASignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {2,IR(0, 0),IR(17742, 27)/* MatrixBSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {4,IR(0, 0),IR(17769, 27)/* MatrixCSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {8,IR(0, 0),IR(17796, 32)/* MatrixResultSignedComponentsKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {16,IR(0, 0),IR(17828, 26)/* SaturatingAccumulationKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {1,IR(0, 0),IR(17854, 4)/* Row */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {2,IR(0, 0),IR(17858, 7)/* Column */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {4,IR(0, 0),IR(17865, 4)/* 2x2 */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {0,IR(0, 0),IR(17869, 11)/* MatrixAKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {1,IR(0, 0),IR(17880, 11)/* MatrixBKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {2,IR(0, 0),IR(17891, 21)/* MatrixAccumulatorKHR */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {0,IR(0, 0),IR(17912, 11)/* RowMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {1,IR(0, 0),IR(17923, 14)/* ColumnMajorNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {2,IR(0, 0),IR(17937, 21)/* InferencingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {3,IR(0, 0),IR(17958, 18)/* TrainingOptimalNV */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {0,IR(0, 0),IR(16824, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16836, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16844, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16852, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16858, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16865, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16876, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {8,IR(0, 0),IR(16885, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16898, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16904, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16914, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16960, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16976, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16990, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(17003, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(17015, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(17032, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(17044, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(17059, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(17072, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(17087, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(17105, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(17122, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(17143, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17163, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17183, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17259, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {1,IR(0, 0),IR(17265, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {2,IR(0, 0),IR(17270, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {3,IR(0, 1),IR(17309, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {4,IR(1, 2),IR(17320, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {5,IR(0, 0),IR(17329, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {6,IR(0, 0),IR(17334, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {7,IR(0, 0),IR(17341, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {8,IR(0, 1),IR(17352, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_OPERATION
  {0,IR(0, 0),IR(17368, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17378, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17391, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(17976, 17)/* RelaxedPrecision */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {1,IR(0, 1),IR(17993, 7)/* SpecId */,IR(0, 0),IR(120, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {2,IR(0, 0),IR(18000, 6)/* Block */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {3,IR(0, 0),IR(18006, 12)/* BufferBlock */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),SPV_SPIRV_VERSION_WORD(1,3)}, // SPV_OPERAND_TYPE_DECORATION
  {4,IR(0, 0),IR(18018, 9)/* RowMajor */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5,IR(0, 0),IR(18027, 9)/* ColMajor */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6,IR(0, 1),IR(18036, 12)/* ArrayStride */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7,IR(0, 1),IR(18048, 13)/* MatrixStride */,IR(0, 0),IR(68, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {8,IR(0, 0),IR(18061, 11)/* GLSLShared */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {9,IR(0, 0),IR(18072, 11)/* GLSLPacked */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {10,IR(0, 0),IR(18083, 8)/* CPacked */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {11,IR(3, 1),IR(18117, 8)/* BuiltIn */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {13,IR(0, 0),IR(18125, 14)/* NoPerspective */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {14,IR(0, 0),IR(18139, 5)/* Flat */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {15,IR(0, 0),IR(18144, 6)/* Patch */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {16,IR(0, 0),IR(18150, 9)/* Centroid */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {17,IR(0, 0),IR(18159, 7)/* Sample */,IR(0, 0),IR(25, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {18,IR(0, 0),IR(18166, 10)/* Invariant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {19,IR(0, 0),IR(18176, 9)/* Restrict */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {20,IR(0, 0),IR(18185, 8)/* Aliased */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {21,IR(0, 0),IR(18193, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {22,IR(0, 0),IR(18202, 9)/* Constant */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {23,IR(0, 0),IR(18211, 9)/* Coherent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {24,IR(0, 0),IR(18220, 12)/* NonWritable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {25,IR(0, 0),IR(18232, 12)/* NonReadable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {26,IR(0, 0),IR(18244, 8)/* Uniform */,IR(0, 0),IR(122, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {27,IR(4, 1),IR(18278, 10)/* UniformId */,IR(0, 0),IR(122, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {28,IR(0, 0),IR(18288, 20)/* SaturatedConversion */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {29,IR(0, 1),IR(18308, 7)/* Stream */,IR(0, 0),IR(124, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {30,IR(0, 1),IR(18315, 9)/* Location */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {31,IR(0, 1),IR(18324, 10)/* Component */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {32,IR(0, 1),IR(18334, 6)/* Index */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {33,IR(0, 1),IR(18340, 8)/* Binding */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {34,IR(0, 1),IR(18348, 14)/* DescriptorSet */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {35,IR(0, 1),IR(18362, 7)/* Offset */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {36,IR(0, 1),IR(18369, 10)/* XfbBuffer */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {37,IR(0, 1),IR(18379, 10)/* XfbStride */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {38,IR(5, 1),IR(18435, 14)/* FuncParamAttr */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {39,IR(6, 1),IR(18483, 15)/* FPRoundingMode */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {40,IR(7, 1),IR(18533, 15)/* FPFastMathMode */,IR(0, 0),IR(126, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {41,IR(8, 2),IR(18610, 18)/* LinkageAttributes */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {42,IR(0, 0),IR(18628, 14)/* NoContraction */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {43,IR(0, 1),IR(18642, 21)/* InputAttachmentIndex */,IR(0, 0),IR(99, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {44,IR(0, 1),IR(18663, 10)/* Alignment */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {45,IR(0, 1),IR(18673, 14)/* MaxByteOffset */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {46,IR(10, 1),IR(18707, 12)/* AlignmentId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {47,IR(10, 1),IR(18719, 16)/* MaxByteOffsetId */,IR(0, 0),IR(1, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4216,IR(0, 0),IR(18735, 44)/* SaturatedToLargestFloat8NormalConversionEXT */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4469,IR(0, 0),IR(18779, 13)/* NoSignedWrap */,IR(0, 0),IR(1, 0),IR(202, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4470,IR(0, 0),IR(18792, 15)/* NoUnsignedWrap */,IR(0, 0),IR(1, 0),IR(202, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4487,IR(0, 0),IR(18807, 18)/* WeightTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4488,IR(0, 0),IR(18825, 22)/* BlockMatchTextureQCOM */,IR(0, 0),IR(1, 0),IR(53, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4499,IR(0, 0),IR(18847, 22)/* BlockMatchSamplerQCOM */,IR(0, 0),IR(1, 0),IR(56, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {4999,IR(0, 0),IR(18869, 18)/* ExplicitInterpAMD */,IR(0, 0),IR(1, 0),IR(10, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5019,IR(10, 1),IR(18887, 32)/* NodeSharesPayloadLimitsWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5020,IR(10, 1),IR(18919, 20)/* NodeMaxPayloadsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5078,IR(0, 0),IR(18939, 23)/* TrackFinishWritingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5091,IR(10, 1),IR(18962, 20)/* PayloadNodeNameAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5098,IR(10, 1),IR(18982, 25)/* PayloadNodeBaseIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5099,IR(0, 0),IR(19007, 27)/* PayloadNodeSparseArrayAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5100,IR(10, 1),IR(19034, 25)/* PayloadNodeArraySizeAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5105,IR(0, 0),IR(19059, 28)/* PayloadDispatchIndirectAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5124,IR(10, 1),IR(19087, 17)/* ArrayStrideIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5125,IR(10, 1),IR(19104, 12)/* OffsetIdEXT */,IR(0, 0),IR(44, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5145,IR(0, 0),IR(19116, 14)/* UTFEncodedKHR */,IR(0, 0),IR(128, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5248,IR(0, 0),IR(19130, 19)/* OverrideCoverageNV */,IR(0, 0),IR(129, 1),IR(76, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5250,IR(0, 0),IR(19149, 14)/* PassthroughNV */,IR(0, 0),IR(130, 1),IR(77, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5252,IR(0, 0),IR(19163, 19)/* ViewportRelativeNV */,IR(0, 0),IR(98, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5256,IR(0, 1),IR(19182, 28)/* SecondaryViewportRelativeNV */,IR(0, 0),IR(47, 1),IR(14, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5271,IR(0, 0),IR(19210, 16)/* PerPrimitiveEXT */,IR(77, 1),IR(131, 2),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5272,IR(0, 0),IR(19241, 10)/* PerViewNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5273,IR(0, 0),IR(19251, 10)/* PerTaskNV */,IR(0, 0),IR(51, 1),IR(18, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5285,IR(0, 0),IR(19261, 13)/* PerVertexKHR */,IR(78, 1),IR(52, 1),IR(19, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5300,IR(0, 0),IR(19286, 11)/* NonUniform */,IR(79, 1),IR(101, 1),IR(87, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5355,IR(0, 0),IR(19311, 16)/* RestrictPointer */,IR(80, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5356,IR(0, 0),IR(19346, 15)/* AliasedPointer */,IR(81, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5358,IR(0, 1),IR(19379, 15)/* MemberOffsetNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5386,IR(0, 0),IR(19394, 30)/* HitObjectShaderRecordBufferNV */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5389,IR(0, 0),IR(19424, 31)/* HitObjectShaderRecordBufferEXT */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5397,IR(0, 1),IR(19455, 7)/* BankNV */,IR(0, 0),IR(133, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5398,IR(0, 0),IR(19462, 18)/* BindlessSamplerNV */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5399,IR(0, 0),IR(19480, 16)/* BindlessImageNV */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5400,IR(0, 0),IR(19496, 15)/* BoundSamplerNV */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5401,IR(0, 0),IR(19511, 13)/* BoundImageNV */,IR(0, 0),IR(136, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5440,IR(0, 0),IR(19524, 30)/* CooperativeMatrixTransposeEXT */,IR(0, 0),IR(137, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5599,IR(0, 1),IR(19554, 14)/* SIMTCallINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5602,IR(0, 0),IR(19568, 26)/* ReferencedIndirectlyINTEL */,IR(0, 0),IR(139, 1),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5607,IR(11, 1),IR(19594, 13)/* ClobberINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5608,IR(0, 0),IR(19607, 17)/* SideEffectsINTEL */,IR(0, 0),IR(140, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5624,IR(0, 0),IR(19624, 27)/* VectorComputeVariableINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5625,IR(0, 1),IR(19651, 21)/* FuncParamIOKindINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5626,IR(0, 0),IR(19672, 27)/* VectorComputeFunctionINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5627,IR(0, 0),IR(19699, 15)/* StackCallINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5628,IR(0, 1),IR(19714, 26)/* GlobalVariableOffsetINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5634,IR(10, 1),IR(19740, 14)/* CounterBuffer */,IR(82, 1),IR(1, 0),IR(205, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5635,IR(11, 1),IR(19778, 13)/* UserSemantic */,IR(83, 1),IR(1, 0),IR(205, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5636,IR(11, 1),IR(19810, 15)/* UserTypeGOOGLE */,IR(0, 0),IR(1, 0),IR(206, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5822,IR(12, 2),IR(19825, 26)/* FunctionRoundingModeINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5823,IR(14, 2),IR(19882, 24)/* FunctionDenormModeINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5825,IR(0, 0),IR(19906, 15)/* RegisterALTERA */,IR(84, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5826,IR(11, 1),IR(19935, 13)/* MemoryALTERA */,IR(85, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5827,IR(0, 1),IR(19960, 15)/* NumbanksALTERA */,IR(86, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5828,IR(0, 1),IR(19989, 16)/* BankwidthALTERA */,IR(87, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5829,IR(0, 1),IR(20020, 23)/* MaxPrivateCopiesALTERA */,IR(88, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5830,IR(0, 0),IR(20065, 17)/* SinglepumpALTERA */,IR(89, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5831,IR(0, 0),IR(20098, 17)/* DoublepumpALTERA */,IR(90, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5832,IR(0, 1),IR(20131, 20)/* MaxReplicatesALTERA */,IR(91, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5833,IR(0, 0),IR(20170, 21)/* SimpleDualPortALTERA */,IR(92, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5834,IR(16, 2),IR(20211, 12)/* MergeALTERA */,IR(93, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5835,IR(18, 1),IR(20276, 15)/* BankBitsALTERA */,IR(94, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5836,IR(0, 1),IR(20305, 21)/* ForcePow2DepthALTERA */,IR(95, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5883,IR(0, 1),IR(20346, 17)/* StridesizeALTERA */,IR(96, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5884,IR(0, 1),IR(20379, 15)/* WordsizeALTERA */,IR(97, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5885,IR(0, 0),IR(20408, 19)/* TrueDualPortALTERA */,IR(98, 1),IR(142, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5899,IR(0, 0),IR(20445, 20)/* BurstCoalesceALTERA */,IR(99, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5900,IR(0, 1),IR(20484, 16)/* CacheSizeALTERA */,IR(100, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5901,IR(0, 0),IR(20515, 29)/* DontStaticallyCoalesceALTERA */,IR(101, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5902,IR(0, 1),IR(20572, 15)/* PrefetchALTERA */,IR(102, 1),IR(143, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5905,IR(0, 0),IR(20601, 18)/* StallEnableALTERA */,IR(103, 1),IR(111, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5907,IR(0, 0),IR(20636, 26)/* FuseLoopsInFunctionALTERA */,IR(104, 1),IR(144, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5909,IR(1, 2),IR(20687, 20)/* MathOpDSPModeALTERA */,IR(105, 1),IR(145, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5914,IR(10, 1),IR(20726, 16)/* AliasScopeINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5915,IR(10, 1),IR(20742, 13)/* NoAliasINTEL */,IR(0, 0),IR(146, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5917,IR(0, 1),IR(20755, 25)/* InitiationIntervalALTERA */,IR(106, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5918,IR(0, 1),IR(20804, 21)/* MaxConcurrencyALTERA */,IR(107, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5919,IR(0, 1),IR(20845, 21)/* PipelineEnableALTERA */,IR(108, 1),IR(147, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5921,IR(0, 1),IR(20886, 21)/* BufferLocationALTERA */,IR(109, 1),IR(148, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {5944,IR(0, 1),IR(20927, 20)/* IOPipeStorageALTERA */,IR(110, 1),IR(149, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6080,IR(19, 2),IR(21000, 31)/* FunctionFloatingPointModeINTEL */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6085,IR(0, 0),IR(21031, 25)/* SingleElementVectorINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6087,IR(0, 0),IR(21056, 35)/* VectorComputeCallableFunctionINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6140,IR(0, 0),IR(21091, 18)/* MediaBlockIOINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6151,IR(0, 0),IR(21109, 16)/* StallFreeALTERA */,IR(111, 1),IR(150, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6170,IR(21, 1),IR(21171, 26)/* FPMaxErrorDecorationINTEL */,IR(0, 0),IR(151, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6172,IR(0, 1),IR(21197, 26)/* LatencyControlLabelALTERA */,IR(112, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6173,IR(22, 3),IR(21248, 31)/* LatencyControlConstraintALTERA */,IR(113, 1),IR(152, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6175,IR(0, 0),IR(21309, 28)/* ConduitKernelArgumentALTERA */,IR(114, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6176,IR(0, 0),IR(21364, 32)/* RegisterMapKernelArgumentALTERA */,IR(115, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6177,IR(0, 1),IR(21427, 34)/* MMHostInterfaceAddressWidthALTERA */,IR(116, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6178,IR(0, 1),IR(21494, 31)/* MMHostInterfaceDataWidthALTERA */,IR(117, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6179,IR(0, 1),IR(21555, 29)/* MMHostInterfaceLatencyALTERA */,IR(118, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6180,IR(25, 1),IR(21646, 35)/* MMHostInterfaceReadWriteModeALTERA */,IR(119, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6181,IR(0, 1),IR(21715, 30)/* MMHostInterfaceMaxBurstALTERA */,IR(120, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6182,IR(0, 1),IR(21774, 33)/* MMHostInterfaceWaitRequestALTERA */,IR(121, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6183,IR(0, 0),IR(21839, 27)/* StableKernelArgumentALTERA */,IR(122, 1),IR(153, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6188,IR(26, 2),IR(21931, 16)/* HostAccessINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6190,IR(28, 1),IR(21994, 15)/* InitModeALTERA */,IR(123, 1),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6191,IR(0, 1),IR(22023, 29)/* ImplementInRegisterMapALTERA */,IR(124, 1),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6247,IR(10, 1),IR(22080, 17)/* ConditionalINTEL */,IR(0, 0),IR(116, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6442,IR(29, 2),IR(22133, 22)/* CacheControlLoadINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {6443,IR(31, 2),IR(22192, 23)/* CacheControlStoreINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {7040,IR(0, 1),IR(16807, 17)/* IntrinsicSAMSUNG */,IR(0, 0),IR(157, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DECORATION
  {0,IR(0, 0),IR(22215, 3)/* 1D */,IR(0, 0),IR(74, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {1,IR(0, 0),IR(22218, 3)/* 2D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {2,IR(0, 0),IR(22221, 3)/* 3D */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {3,IR(0, 0),IR(22224, 5)/* Cube */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4,IR(0, 0),IR(22229, 5)/* Rect */,IR(0, 0),IR(73, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {5,IR(0, 0),IR(22234, 7)/* Buffer */,IR(0, 0),IR(75, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {6,IR(0, 0),IR(22241, 12)/* SubpassData */,IR(0, 0),IR(99, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {4173,IR(0, 0),IR(22253, 17)/* TileImageDataEXT */,IR(0, 0),IR(158, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_DIMENSIONALITY
  {0,IR(0, 1),IR(22270, 12)/* Invocations */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {1,IR(0, 0),IR(22282, 13)/* SpacingEqual */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {2,IR(0, 0),IR(22295, 22)/* SpacingFractionalEven */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {3,IR(0, 0),IR(22317, 21)/* SpacingFractionalOdd */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4,IR(0, 0),IR(22338, 14)/* VertexOrderCw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5,IR(0, 0),IR(22352, 15)/* VertexOrderCcw */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6,IR(0, 0),IR(22367, 19)/* PixelCenterInteger */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {7,IR(0, 0),IR(22386, 16)/* OriginUpperLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {8,IR(0, 0),IR(22402, 16)/* OriginLowerLeft */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {9,IR(0, 0),IR(22418, 19)/* EarlyFragmentTests */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {10,IR(0, 0),IR(22437, 10)/* PointMode */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {11,IR(0, 0),IR(22447, 4)/* Xfb */,IR(0, 0),IR(125, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {12,IR(0, 0),IR(22451, 15)/* DepthReplacing */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {14,IR(0, 0),IR(22466, 13)/* DepthGreater */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {15,IR(0, 0),IR(22479, 10)/* DepthLess */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {16,IR(0, 0),IR(22489, 15)/* DepthUnchanged */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {17,IR(22, 3),IR(22504, 10)/* LocalSize */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {18,IR(22, 3),IR(22514, 14)/* LocalSizeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {19,IR(0, 0),IR(22528, 12)/* InputPoints */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {20,IR(0, 0),IR(22540, 11)/* InputLines */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {21,IR(0, 0),IR(22551, 20)/* InputLinesAdjacency */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {22,IR(0, 0),IR(22571, 10)/* Triangles */,IR(0, 0),IR(12, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {23,IR(0, 0),IR(22581, 24)/* InputTrianglesAdjacency */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {24,IR(0, 0),IR(22605, 6)/* Quads */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {25,IR(0, 0),IR(22611, 9)/* Isolines */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {26,IR(0, 1),IR(22620, 15)/* OutputVertices */,IR(0, 0),IR(159, 4),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {27,IR(0, 0),IR(22635, 13)/* OutputPoints */,IR(0, 0),IR(163, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {28,IR(0, 0),IR(22648, 16)/* OutputLineStrip */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {29,IR(0, 0),IR(22664, 20)/* OutputTriangleStrip */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {30,IR(0, 1),IR(22684, 12)/* VecTypeHint */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {31,IR(0, 0),IR(22696, 15)/* ContractionOff */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {33,IR(0, 0),IR(22711, 12)/* Initializer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {34,IR(0, 0),IR(22723, 10)/* Finalizer */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {35,IR(0, 1),IR(6692, 13)/* SubgroupSize */,IR(0, 0),IR(166, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {36,IR(0, 1),IR(22733, 22)/* SubgroupsPerWorkgroup */,IR(0, 0),IR(166, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {37,IR(10, 1),IR(22755, 24)/* SubgroupsPerWorkgroupId */,IR(0, 0),IR(166, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {38,IR(33, 3),IR(22779, 12)/* LocalSizeId */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {39,IR(33, 3),IR(22791, 16)/* LocalSizeHintId */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,2),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4169,IR(0, 0),IR(22807, 34)/* NonCoherentColorAttachmentReadEXT */,IR(0, 0),IR(158, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4170,IR(0, 0),IR(22841, 34)/* NonCoherentDepthAttachmentReadEXT */,IR(0, 0),IR(167, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4171,IR(0, 0),IR(22875, 36)/* NonCoherentStencilAttachmentReadEXT */,IR(0, 0),IR(168, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4421,IR(0, 0),IR(22911, 30)/* SubgroupUniformControlFlowKHR */,IR(0, 0),IR(3, 1),IR(207, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4446,IR(0, 0),IR(22941, 18)/* PostDepthCoverage */,IR(0, 0),IR(169, 1),IR(46, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4459,IR(0, 1),IR(11053, 15)/* DenormPreserve */,IR(0, 0),IR(170, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4460,IR(0, 1),IR(11068, 18)/* DenormFlushToZero */,IR(0, 0),IR(171, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4461,IR(0, 1),IR(11086, 25)/* SignedZeroInfNanPreserve */,IR(0, 0),IR(172, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4462,IR(0, 1),IR(11111, 16)/* RoundingModeRTE */,IR(0, 0),IR(173, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4463,IR(0, 1),IR(11127, 16)/* RoundingModeRTZ */,IR(0, 0),IR(174, 1),IR(48, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4489,IR(0, 0),IR(22959, 34)/* NonCoherentTileAttachmentReadQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4490,IR(22, 3),IR(22993, 20)/* TileShadingRateQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4507,IR(0, 0),IR(23013, 21)/* SubgroupSizeHalfQCOM */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {4508,IR(0, 0),IR(23034, 21)/* SubgroupSizeFullQCOM */,IR(0, 0),IR(175, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5017,IR(0, 0),IR(23055, 29)/* EarlyAndLateFragmentTestsAMD */,IR(0, 0),IR(3, 1),IR(208, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5027,IR(0, 0),IR(23084, 23)/* StencilRefReplacingEXT */,IR(0, 0),IR(42, 1),IR(11, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5069,IR(0, 0),IR(23107, 15)/* CoalescingAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5070,IR(10, 1),IR(23122, 15)/* IsApiEntryAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5071,IR(10, 1),IR(23137, 21)/* MaxNodeRecursionAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5072,IR(33, 3),IR(23158, 24)/* StaticNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5073,IR(10, 1),IR(7585, 16)/* ShaderIndexAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5077,IR(33, 3),IR(23182, 21)/* MaxNumWorkgroupsAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5079,IR(0, 0),IR(23203, 28)/* StencilRefUnchangedFrontAMD */,IR(0, 0),IR(42, 1),IR(209, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5080,IR(0, 0),IR(23231, 26)/* StencilRefGreaterFrontAMD */,IR(0, 0),IR(42, 1),IR(209, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5081,IR(0, 0),IR(23257, 23)/* StencilRefLessFrontAMD */,IR(0, 0),IR(42, 1),IR(209, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5082,IR(0, 0),IR(23280, 27)/* StencilRefUnchangedBackAMD */,IR(0, 0),IR(42, 1),IR(209, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5083,IR(0, 0),IR(23307, 25)/* StencilRefGreaterBackAMD */,IR(0, 0),IR(42, 1),IR(209, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5084,IR(0, 0),IR(23332, 22)/* StencilRefLessBackAMD */,IR(0, 0),IR(42, 1),IR(209, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5088,IR(0, 0),IR(23354, 19)/* QuadDerivativesKHR */,IR(0, 0),IR(176, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5089,IR(0, 0),IR(23373, 20)/* RequireFullQuadsKHR */,IR(0, 0),IR(176, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5102,IR(36, 2),IR(23393, 20)/* SharesInputWithAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5157,IR(0, 0),IR(23413, 20)/* ArithmeticPoisonKHR */,IR(0, 0),IR(177, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5269,IR(0, 0),IR(23433, 15)/* OutputLinesEXT */,IR(125, 1),IR(131, 2),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5270,IR(0, 1),IR(23462, 20)/* OutputPrimitivesEXT */,IR(126, 1),IR(131, 2),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5289,IR(0, 0),IR(23501, 24)/* DerivativeGroupQuadsKHR */,IR(127, 1),IR(178, 1),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5290,IR(0, 0),IR(23548, 25)/* DerivativeGroupLinearKHR */,IR(128, 1),IR(179, 1),IR(83, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5298,IR(0, 0),IR(23597, 19)/* OutputTrianglesEXT */,IR(129, 1),IR(131, 2),IR(203, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5366,IR(0, 0),IR(23634, 25)/* PixelInterlockOrderedEXT */,IR(0, 0),IR(180, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5367,IR(0, 0),IR(23659, 27)/* PixelInterlockUnorderedEXT */,IR(0, 0),IR(180, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5368,IR(0, 0),IR(23686, 26)/* SampleInterlockOrderedEXT */,IR(0, 0),IR(181, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5369,IR(0, 0),IR(23712, 28)/* SampleInterlockUnorderedEXT */,IR(0, 0),IR(181, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5370,IR(0, 0),IR(23740, 31)/* ShadingRateInterlockOrderedEXT */,IR(0, 0),IR(182, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5371,IR(0, 0),IR(23771, 33)/* ShadingRateInterlockUnorderedEXT */,IR(0, 0),IR(182, 1),IR(91, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5427,IR(0, 0),IR(13554, 23)/* Shader64BitIndexingEXT */,IR(0, 0),IR(183, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5618,IR(0, 1),IR(23804, 27)/* SharedLocalMemorySizeINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5620,IR(0, 1),IR(23831, 21)/* RoundingModeRTPINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5621,IR(0, 1),IR(23852, 21)/* RoundingModeRTNINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5622,IR(0, 1),IR(23873, 26)/* FloatingPointModeALTINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5623,IR(0, 1),IR(23899, 27)/* FloatingPointModeIEEEINTEL */,IR(0, 0),IR(184, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5893,IR(22, 3),IR(23926, 22)/* MaxWorkgroupSizeINTEL */,IR(0, 0),IR(185, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5894,IR(0, 1),IR(23948, 16)/* MaxWorkDimINTEL */,IR(0, 0),IR(185, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5895,IR(0, 0),IR(23964, 20)/* NoGlobalOffsetINTEL */,IR(0, 0),IR(185, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5896,IR(0, 1),IR(23984, 22)/* NumSIMDWorkitemsINTEL */,IR(0, 0),IR(112, 1),IR(132, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {5903,IR(0, 1),IR(24006, 28)/* SchedulerTargetFmaxMhzINTEL */,IR(0, 0),IR(112, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6023,IR(0, 0),IR(24034, 24)/* MaximallyReconvergesKHR */,IR(0, 0),IR(3, 1),IR(211, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6028,IR(36, 2),IR(24058, 18)/* FPFastMathDefault */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6031,IR(10, 1),IR(24076, 21)/* OpacityMicromapIdKHR */,IR(0, 0),IR(187, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6154,IR(0, 1),IR(24097, 24)/* StreamingInterfaceINTEL */,IR(0, 0),IR(112, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6160,IR(0, 1),IR(24121, 26)/* RegisterMapInterfaceINTEL */,IR(0, 0),IR(188, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6417,IR(0, 1),IR(24147, 23)/* NamedBarrierCountINTEL */,IR(0, 0),IR(138, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6461,IR(0, 1),IR(24170, 22)/* MaximumRegistersINTEL */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6462,IR(10, 1),IR(24192, 24)/* MaximumRegistersIdINTEL */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {6463,IR(38, 1),IR(24267, 27)/* NamedMaximumRegistersINTEL */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODE
  {0,IR(0, 0),IR(24294, 7)/* Vertex */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {1,IR(0, 0),IR(24301, 20)/* TessellationControl */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {2,IR(0, 0),IR(24321, 23)/* TessellationEvaluation */,IR(0, 0),IR(24, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {3,IR(0, 0),IR(6173, 9)/* Geometry */,IR(0, 0),IR(71, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {4,IR(0, 0),IR(17359, 9)/* Fragment */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5,IR(0, 0),IR(24344, 10)/* GLCompute */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {6,IR(0, 0),IR(5940, 7)/* Kernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5267,IR(0, 0),IR(24354, 7)/* TaskNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5268,IR(0, 0),IR(24361, 7)/* MeshNV */,IR(0, 0),IR(51, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5313,IR(0, 0),IR(24368, 17)/* RayGenerationKHR */,IR(130, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5314,IR(0, 0),IR(24401, 16)/* IntersectionKHR */,IR(131, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5315,IR(0, 0),IR(24432, 10)/* AnyHitKHR */,IR(132, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5316,IR(0, 0),IR(24451, 14)/* ClosestHitKHR */,IR(133, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5317,IR(0, 0),IR(24478, 8)/* MissKHR */,IR(134, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5318,IR(0, 0),IR(24493, 12)/* CallableKHR */,IR(135, 1),IR(55, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5364,IR(0, 0),IR(24516, 8)/* TaskEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {5365,IR(0, 0),IR(24524, 8)/* MeshEXT */,IR(0, 0),IR(54, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_EXECUTION_MODEL
  {0,IR(0, 0),IR(24532, 9)/* Preserve */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {1,IR(0, 0),IR(24541, 12)/* FlushToZero */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPDENORM_MODE
  {0,IR(0, 0),IR(24553, 12)/* BFloat16KHR */,IR(0, 0),IR(91, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4214,IR(0, 0),IR(24565, 14)/* Float8E4M3EXT */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4215,IR(0, 0),IR(24579, 14)/* Float8E5M2EXT */,IR(0, 0),IR(118, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4223,IR(0, 0),IR(24593, 14)/* Float6E2M3EXT */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4224,IR(0, 0),IR(24607, 14)/* Float6E3M2EXT */,IR(0, 0),IR(190, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4225,IR(0, 0),IR(24621, 14)/* Float4E2M1EXT */,IR(0, 0),IR(191, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4226,IR(0, 0),IR(10548, 22)/* Float8UnsignedE8M0EXT */,IR(0, 0),IR(192, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {4227,IR(0, 0),IR(10570, 10)/* MXInt8EXT */,IR(0, 0),IR(193, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPENCODING
  {0,IR(0, 0),IR(24635, 5)/* IEEE */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {1,IR(0, 0),IR(24640, 4)/* ALT */,IR(0, 0),IR(141, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FPOPERATION_MODE
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {1,IR(0, 0),IR(24644, 7)/* NotNaN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {2,IR(0, 0),IR(24651, 7)/* NotInf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {4,IR(0, 0),IR(24658, 4)/* NSZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {8,IR(0, 0),IR(24662, 11)/* AllowRecip */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {16,IR(0, 0),IR(24673, 5)/* Fast */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {65536,IR(0, 0),IR(24678, 14)/* AllowContract */,IR(136, 1),IR(194, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {131072,IR(0, 0),IR(24715, 13)/* AllowReassoc */,IR(137, 1),IR(194, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {262144,IR(0, 0),IR(24746, 15)/* AllowTransform */,IR(0, 0),IR(186, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {0,IR(0, 0),IR(24761, 4)/* RTE */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(24765, 4)/* RTZ */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {2,IR(0, 0),IR(24769, 4)/* RTP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {3,IR(0, 0),IR(24773, 4)/* RTN */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {1,IR(0, 0),IR(24777, 16)/* Vertical2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {2,IR(0, 0),IR(24793, 16)/* Vertical4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {4,IR(0, 0),IR(24809, 18)/* Horizontal2Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {8,IR(0, 0),IR(24827, 18)/* Horizontal4Pixels */,IR(0, 0),IR(38, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {1,IR(0, 0),IR(24845, 7)/* Inline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {2,IR(0, 0),IR(24852, 11)/* DontInline */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {4,IR(0, 0),IR(24863, 5)/* Pure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {8,IR(0, 0),IR(24868, 6)/* Const */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {65536,IR(0, 0),IR(15763, 11)/* OptNoneEXT */,IR(70, 1),IR(196, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {0,IR(0, 0),IR(24874, 5)/* Zext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {1,IR(0, 0),IR(24879, 5)/* Sext */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {2,IR(0, 0),IR(24884, 6)/* ByVal */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {3,IR(0, 0),IR(24890, 5)/* Sret */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {4,IR(0, 0),IR(24895, 8)/* NoAlias */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5,IR(0, 0),IR(24903, 10)/* NoCapture */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {6,IR(0, 0),IR(24913, 8)/* NoWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {7,IR(0, 0),IR(24921, 12)/* NoReadWrite */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {5940,IR(0, 0),IR(24933, 21)/* RuntimeAlignedALTERA */,IR(138, 1),IR(197, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {0,IR(0, 0),IR(24974, 14)/* Gather4x1QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {1,IR(0, 0),IR(24988, 12)/* GatherDQCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {2,IR(0, 0),IR(25000, 13)/* GatherH2QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {3,IR(0, 0),IR(25013, 13)/* GatherV2QCOM */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GATHER_MODES
  {0,IR(0, 0),IR(25026, 7)/* Reduce */,IR(0, 0),IR(198, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {1,IR(0, 0),IR(25033, 14)/* InclusiveScan */,IR(0, 0),IR(198, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {2,IR(0, 0),IR(25047, 14)/* ExclusiveScan */,IR(0, 0),IR(198, 3),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {3,IR(0, 0),IR(25061, 16)/* ClusteredReduce */,IR(0, 0),IR(201, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {6,IR(0, 0),IR(25077, 21)/* PartitionedReduceEXT */,IR(139, 1),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {7,IR(0, 0),IR(25118, 28)/* PartitionedInclusiveScanEXT */,IR(140, 1),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {8,IR(0, 0),IR(25173, 28)/* PartitionedExclusiveScanEXT */,IR(141, 1),IR(202, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_GROUP_OPERATION
  {0,IR(0, 0),IR(25228, 10)/* NoneINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {1,IR(0, 0),IR(25238, 10)/* ReadINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {2,IR(0, 0),IR(25248, 11)/* WriteINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {3,IR(0, 0),IR(25259, 15)/* ReadWriteINTEL */,IR(0, 0),IR(154, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1,IR(10, 1),IR(25274, 5)/* Bias */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2,IR(10, 1),IR(25279, 4)/* Lod */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4,IR(36, 2),IR(25283, 5)/* Grad */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8,IR(10, 1),IR(25288, 12)/* ConstOffset */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16,IR(10, 1),IR(18362, 7)/* Offset */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {32,IR(10, 1),IR(25300, 13)/* ConstOffsets */,IR(0, 0),IR(203, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {64,IR(10, 1),IR(18159, 7)/* Sample */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {128,IR(10, 1),IR(9816, 7)/* MinLod */,IR(0, 0),IR(204, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {256,IR(4, 1),IR(25313, 19)/* MakeTexelAvailable */,IR(142, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {512,IR(4, 1),IR(25354, 17)/* MakeTexelVisible */,IR(143, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {1024,IR(0, 0),IR(25391, 16)/* NonPrivateTexel */,IR(144, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {2048,IR(0, 0),IR(25426, 14)/* VolatileTexel */,IR(145, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {4096,IR(0, 0),IR(25457, 11)/* SignExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {8192,IR(0, 0),IR(25468, 11)/* ZeroExtend */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {16384,IR(0, 0),IR(25479, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {65536,IR(10, 1),IR(25491, 8)/* Offsets */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE
  {0,IR(0, 0),IR(25499, 10)/* SnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {1,IR(0, 0),IR(25509, 11)/* SnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {2,IR(0, 0),IR(25520, 10)/* UnormInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {3,IR(0, 0),IR(25530, 11)/* UnormInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {4,IR(0, 0),IR(25541, 14)/* UnormShort565 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {5,IR(0, 0),IR(25555, 14)/* UnormShort555 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {6,IR(0, 0),IR(25569, 15)/* UnormInt101010 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {7,IR(0, 0),IR(25584, 11)/* SignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {8,IR(0, 0),IR(25595, 12)/* SignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {9,IR(0, 0),IR(25607, 12)/* SignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {10,IR(0, 0),IR(25619, 13)/* UnsignedInt8 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {11,IR(0, 0),IR(25632, 14)/* UnsignedInt16 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {12,IR(0, 0),IR(25646, 14)/* UnsignedInt32 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {13,IR(0, 0),IR(25660, 10)/* HalfFloat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {14,IR(0, 0),IR(16852, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {15,IR(0, 0),IR(25670, 11)/* UnormInt24 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {16,IR(0, 0),IR(25681, 17)/* UnormInt101010_2 */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {17,IR(0, 0),IR(25698, 16)/* UnormInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {19,IR(0, 0),IR(25714, 20)/* UnsignedIntRaw10EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {20,IR(0, 0),IR(25734, 20)/* UnsignedIntRaw12EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {21,IR(0, 0),IR(25754, 20)/* UnormInt2_101010EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {22,IR(0, 0),IR(25774, 19)/* UnsignedInt10X6EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {23,IR(0, 0),IR(25793, 19)/* UnsignedInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {24,IR(0, 0),IR(25812, 19)/* UnsignedInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {25,IR(0, 0),IR(25831, 16)/* UnormInt12X4EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {26,IR(0, 0),IR(25847, 16)/* UnormInt14X2EXT */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {0,IR(0, 0),IR(25863, 2)/* R */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {1,IR(0, 0),IR(25865, 2)/* A */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {2,IR(0, 0),IR(25867, 3)/* RG */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {3,IR(0, 0),IR(25870, 3)/* RA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {4,IR(0, 0),IR(25873, 4)/* RGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {5,IR(0, 0),IR(25877, 5)/* RGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {6,IR(0, 0),IR(25882, 5)/* BGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {7,IR(0, 0),IR(25887, 5)/* ARGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {8,IR(0, 0),IR(25892, 10)/* Intensity */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {9,IR(0, 0),IR(25902, 10)/* Luminance */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {10,IR(0, 0),IR(25912, 3)/* Rx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {11,IR(0, 0),IR(25915, 4)/* RGx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {12,IR(0, 0),IR(25919, 5)/* RGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {13,IR(0, 0),IR(25924, 6)/* Depth */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {14,IR(0, 0),IR(25930, 13)/* DepthStencil */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {15,IR(0, 0),IR(25943, 5)/* sRGB */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {16,IR(0, 0),IR(25948, 6)/* sRGBx */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {17,IR(0, 0),IR(25954, 6)/* sRGBA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {18,IR(0, 0),IR(25960, 6)/* sBGRA */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {19,IR(0, 0),IR(25966, 5)/* ABGR */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {0,IR(0, 0),IR(25971, 28)/* InitOnDeviceReprogramALTERA */,IR(146, 1),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {1,IR(0, 0),IR(26026, 24)/* InitOnDeviceResetALTERA */,IR(147, 1),IR(155, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {0,IR(0, 0),IR(26073, 7)/* NoWait */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {1,IR(0, 0),IR(26080, 11)/* WaitKernel */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {2,IR(0, 0),IR(26091, 14)/* WaitWorkGroup */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {1,IR(0, 0),IR(26105, 12)/* CmdExecTime */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {1,IR(0, 0),IR(26117, 13)/* MayUsePrintf */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {0,IR(0, 0),IR(26130, 7)/* Export */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {1,IR(0, 0),IR(26137, 7)/* Import */,IR(0, 0),IR(95, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {2,IR(0, 0),IR(26144, 12)/* LinkOnceODR */,IR(0, 0),IR(95, 1),IR(212, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {3,IR(0, 0),IR(26156, 8)/* WeakAMD */,IR(0, 0),IR(206, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LINKAGE_TYPE
  {0,IR(0, 0),IR(26164, 14)/* UncachedINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {1,IR(0, 0),IR(26178, 12)/* CachedINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {2,IR(0, 0),IR(26190, 15)/* StreamingINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {3,IR(0, 0),IR(26205, 25)/* InvalidateAfterReadINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {4,IR(0, 0),IR(26230, 17)/* ConstCachedINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1,IR(0, 0),IR(26247, 7)/* Unroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2,IR(0, 0),IR(26254, 11)/* DontUnroll */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4,IR(0, 0),IR(26265, 19)/* DependencyInfinite */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8,IR(0, 1),IR(26284, 17)/* DependencyLength */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,1),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16,IR(0, 1),IR(26301, 14)/* MinIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {32,IR(0, 1),IR(26315, 14)/* MaxIterations */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {64,IR(0, 1),IR(26329, 18)/* IterationMultiple */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {128,IR(0, 1),IR(26347, 10)/* PeelCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {256,IR(0, 1),IR(26357, 13)/* PartialCount */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {65536,IR(0, 1),IR(20755, 25)/* InitiationIntervalALTERA */,IR(106, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {131072,IR(0, 1),IR(20804, 21)/* MaxConcurrencyALTERA */,IR(107, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {262144,IR(0, 1),IR(26370, 22)/* DependencyArrayALTERA */,IR(148, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {524288,IR(0, 1),IR(20845, 21)/* PipelineEnableALTERA */,IR(108, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {1048576,IR(0, 1),IR(26413, 19)/* LoopCoalesceALTERA */,IR(149, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {2097152,IR(0, 1),IR(26450, 22)/* MaxInterleavingALTERA */,IR(150, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {4194304,IR(0, 1),IR(26493, 27)/* SpeculatedIterationsALTERA */,IR(151, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {8388608,IR(0, 0),IR(26546, 15)/* NoFusionALTERA */,IR(152, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {16777216,IR(0, 1),IR(26575, 16)/* LoopCountALTERA */,IR(153, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {33554432,IR(0, 1),IR(26606, 27)/* MaxReinvocationDelayALTERA */,IR(154, 1),IR(207, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {268435456,IR(0, 1),IR(11389, 23)/* MultipleWaitQueuesQCOM */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_LOOP_CONTROL
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1,IR(0, 0),IR(26659, 29)/* MatrixASignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2,IR(0, 0),IR(26688, 29)/* MatrixBSignedComponentsINTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4,IR(0, 0),IR(26717, 21)/* MatrixCBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8,IR(0, 0),IR(26738, 26)/* MatrixResultBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {16,IR(0, 0),IR(26764, 23)/* MatrixAPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {32,IR(0, 0),IR(26787, 23)/* MatrixBPackedInt8INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {64,IR(0, 0),IR(26810, 23)/* MatrixAPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {128,IR(0, 0),IR(26833, 23)/* MatrixBPackedInt4INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {256,IR(0, 0),IR(26856, 17)/* MatrixATF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {512,IR(0, 0),IR(26873, 17)/* MatrixBTF32INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {1024,IR(0, 0),IR(26890, 26)/* MatrixAPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {2048,IR(0, 0),IR(26916, 26)/* MatrixBPackedFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {4096,IR(0, 0),IR(26942, 27)/* MatrixAPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {8192,IR(0, 0),IR(26969, 27)/* MatrixBPackedBFloat16INTEL */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {1,IR(0, 0),IR(18193, 9)/* Volatile */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {2,IR(0, 1),IR(26996, 8)/* Aligned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {4,IR(0, 0),IR(25479, 12)/* Nontemporal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {8,IR(4, 1),IR(27004, 21)/* MakePointerAvailable */,IR(155, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {16,IR(4, 1),IR(27049, 19)/* MakePointerVisible */,IR(156, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {32,IR(0, 0),IR(27090, 18)/* NonPrivatePointer */,IR(157, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {65536,IR(10, 1),IR(27129, 20)/* AliasScopeINTELMask */,IR(0, 0),IR(146, 1),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {131072,IR(10, 1),IR(27149, 17)/* NoAliasINTELMask */,IR(0, 0),IR(146, 1),IR(141, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_ACCESS
  {0,IR(0, 0),IR(27166, 7)/* Simple */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {1,IR(0, 0),IR(27173, 8)/* GLSL450 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {2,IR(0, 0),IR(27181, 7)/* OpenCL */,IR(0, 0),IR(0, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {3,IR(0, 0),IR(27188, 7)/* Vulkan */,IR(158, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_MODEL
  {0,IR(0, 0),IR(27205, 8)/* Relaxed */,IR(159, 1),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2,IR(0, 0),IR(27213, 8)/* Acquire */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4,IR(0, 0),IR(27221, 8)/* Release */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8,IR(0, 0),IR(27229, 15)/* AcquireRelease */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16,IR(0, 0),IR(27244, 23)/* SequentiallyConsistent */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {64,IR(0, 0),IR(27267, 14)/* UniformMemory */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {128,IR(0, 0),IR(27281, 15)/* SubgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {256,IR(0, 0),IR(27296, 16)/* WorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {512,IR(0, 0),IR(27312, 21)/* CrossWorkgroupMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {1024,IR(0, 0),IR(27333, 20)/* AtomicCounterMemory */,IR(0, 0),IR(84, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {2048,IR(0, 0),IR(27353, 12)/* ImageMemory */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {4096,IR(0, 0),IR(27365, 13)/* OutputMemory */,IR(160, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {8192,IR(0, 0),IR(27394, 14)/* MakeAvailable */,IR(161, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {16384,IR(0, 0),IR(27425, 12)/* MakeVisible */,IR(162, 1),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {32768,IR(0, 0),IR(18193, 9)/* Volatile */,IR(0, 0),IR(205, 1),IR(89, 1),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {0,IR(0, 0),IR(27452, 10)/* AutoINTEL */,IR(0, 0),IR(189, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {0,IR(0, 0),IR(27462, 5)/* WRAP */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {1,IR(0, 0),IR(27467, 4)/* SAT */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {2,IR(0, 0),IR(27471, 9)/* SAT_ZERO */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {3,IR(0, 0),IR(27480, 8)/* SAT_SYM */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_OVERFLOW_MODES
  {0,IR(0, 0),IR(27488, 25)/* PackedVectorFormat4x8Bit */,IR(163, 1),IR(1, 0),IR(158, 1),SPV_SPIRV_VERSION_WORD(1,6),0xffffffffu}, // SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {0,IR(0, 0),IR(27541, 4)/* TRN */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {1,IR(0, 0),IR(27545, 9)/* TRN_ZERO */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {2,IR(0, 0),IR(27554, 4)/* RND */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {3,IR(0, 0),IR(27558, 9)/* RND_ZERO */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {4,IR(0, 0),IR(27567, 8)/* RND_INF */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {5,IR(0, 0),IR(27575, 12)/* RND_MIN_INF */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {6,IR(0, 0),IR(27587, 9)/* RND_CONV */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {7,IR(0, 0),IR(27596, 13)/* RND_CONV_ODD */,IR(0, 0),IR(208, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {1,IR(0, 0),IR(27609, 25)/* RobustnessPerComponentNV */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {2,IR(0, 0),IR(27634, 23)/* RobustnessPerElementNV */,IR(0, 0),IR(209, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {0,IR(0, 0),IR(17707, 8)/* NoneKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1,IR(0, 0),IR(27657, 10)/* OpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {2,IR(0, 0),IR(27667, 12)/* NoOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {4,IR(0, 0),IR(27679, 23)/* TerminateOnFirstHitKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {8,IR(0, 0),IR(27702, 24)/* SkipClosestHitShaderKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {16,IR(0, 0),IR(27726, 27)/* CullBackFacingTrianglesKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {32,IR(0, 0),IR(27753, 28)/* CullFrontFacingTrianglesKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {64,IR(0, 0),IR(27781, 14)/* CullOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {128,IR(0, 0),IR(27795, 16)/* CullNoOpaqueKHR */,IR(0, 0),IR(86, 2),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {256,IR(0, 0),IR(27811, 17)/* SkipTrianglesKHR */,IR(164, 1),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {512,IR(0, 0),IR(27852, 13)/* SkipAABBsKHR */,IR(0, 0),IR(210, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {1024,IR(0, 0),IR(27865, 30)/* ForceOpacityMicromap2StateKHR */,IR(165, 1),IR(211, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_FLAGS
  {0,IR(0, 0),IR(27925, 41)/* RayQueryCandidateIntersectionTriangleKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {1,IR(0, 0),IR(27966, 37)/* RayQueryCandidateIntersectionAABBKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {0,IR(0, 0),IR(28003, 37)/* RayQueryCommittedIntersectionNoneKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {1,IR(0, 0),IR(28040, 41)/* RayQueryCommittedIntersectionTriangleKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {2,IR(0, 0),IR(28081, 42)/* RayQueryCommittedIntersectionGeneratedKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {0,IR(0, 0),IR(28123, 33)/* RayQueryCandidateIntersectionKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {1,IR(0, 0),IR(28156, 33)/* RayQueryCommittedIntersectionKHR */,IR(0, 0),IR(212, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {1,IR(0, 0),IR(28189, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {2,IR(0, 0),IR(28201, 6)/* Clamp */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {3,IR(0, 0),IR(28207, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {4,IR(0, 0),IR(28214, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {0,IR(0, 0),IR(28229, 8)/* Nearest */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {1,IR(0, 0),IR(28237, 7)/* Linear */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {0,IR(0, 0),IR(28244, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {1,IR(0, 0),IR(28252, 8)/* Rgba32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {2,IR(0, 0),IR(28260, 8)/* Rgba16f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {3,IR(0, 0),IR(28268, 5)/* R32f */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {4,IR(0, 0),IR(28273, 6)/* Rgba8 */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {5,IR(0, 0),IR(28279, 11)/* Rgba8Snorm */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {6,IR(0, 0),IR(28290, 6)/* Rg32f */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {7,IR(0, 0),IR(28296, 6)/* Rg16f */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {8,IR(0, 0),IR(28302, 13)/* R11fG11fB10f */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {9,IR(0, 0),IR(28315, 5)/* R16f */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {10,IR(0, 0),IR(28320, 7)/* Rgba16 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {11,IR(0, 0),IR(28327, 8)/* Rgb10A2 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {12,IR(0, 0),IR(28335, 5)/* Rg16 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {13,IR(0, 0),IR(28340, 4)/* Rg8 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {14,IR(0, 0),IR(28344, 4)/* R16 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {15,IR(0, 0),IR(28348, 3)/* R8 */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {16,IR(0, 0),IR(28351, 12)/* Rgba16Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {17,IR(0, 0),IR(28363, 10)/* Rg16Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {18,IR(0, 0),IR(28373, 9)/* Rg8Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {19,IR(0, 0),IR(28382, 9)/* R16Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {20,IR(0, 0),IR(28391, 8)/* R8Snorm */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {21,IR(0, 0),IR(28399, 8)/* Rgba32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {22,IR(0, 0),IR(28407, 8)/* Rgba16i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {23,IR(0, 0),IR(28415, 7)/* Rgba8i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {24,IR(0, 0),IR(28422, 5)/* R32i */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {25,IR(0, 0),IR(28427, 6)/* Rg32i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {26,IR(0, 0),IR(28433, 6)/* Rg16i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {27,IR(0, 0),IR(28439, 5)/* Rg8i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {28,IR(0, 0),IR(28444, 5)/* R16i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {29,IR(0, 0),IR(28449, 4)/* R8i */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {30,IR(0, 0),IR(28453, 9)/* Rgba32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {31,IR(0, 0),IR(28462, 9)/* Rgba16ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {32,IR(0, 0),IR(28471, 8)/* Rgba8ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {33,IR(0, 0),IR(28479, 6)/* R32ui */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {34,IR(0, 0),IR(28485, 10)/* Rgb10a2ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {35,IR(0, 0),IR(28495, 7)/* Rg32ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {36,IR(0, 0),IR(28502, 7)/* Rg16ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {37,IR(0, 0),IR(28509, 6)/* Rg8ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {38,IR(0, 0),IR(28515, 6)/* R16ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {39,IR(0, 0),IR(28521, 5)/* R8ui */,IR(0, 0),IR(213, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {40,IR(0, 0),IR(28526, 6)/* R64ui */,IR(0, 0),IR(214, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {41,IR(0, 0),IR(28532, 5)/* R64i */,IR(0, 0),IR(214, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {0,IR(0, 0),IR(28537, 12)/* CrossDevice */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {1,IR(0, 0),IR(28549, 7)/* Device */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {2,IR(0, 0),IR(28556, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {3,IR(0, 0),IR(28566, 9)/* Subgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {4,IR(0, 0),IR(28575, 11)/* Invocation */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {5,IR(0, 0),IR(28586, 12)/* QueueFamily */,IR(166, 1),IR(205, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {6,IR(0, 0),IR(28613, 14)/* ShaderCallKHR */,IR(0, 0),IR(61, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_SCOPE_ID
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(28627, 8)/* Flatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {2,IR(0, 0),IR(28635, 12)/* DontFlatten */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SELECTION_CONTROL
  {1,IR(0, 0),IR(28647, 29)/* IdentifierPossibleDuplicates */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {0,IR(0, 0),IR(16824, 12)/* Unspecified */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {1,IR(0, 0),IR(16836, 8)/* Address */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {2,IR(0, 0),IR(16844, 8)/* Boolean */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {3,IR(0, 0),IR(16852, 6)/* Float */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {4,IR(0, 0),IR(16858, 7)/* Signed */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {5,IR(0, 0),IR(16865, 11)/* SignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {6,IR(0, 0),IR(16876, 9)/* Unsigned */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {7,IR(0, 0),IR(16885, 13)/* UnsignedChar */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {0,IR(0, 0),IR(16898, 6)/* Class */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {1,IR(0, 0),IR(16904, 10)/* Structure */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {2,IR(0, 0),IR(16914, 6)/* Union */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {0,IR(0, 0),IR(16920, 15)/* ImportedModule */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {1,IR(0, 0),IR(16935, 20)/* ImportedDeclaration */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1,IR(0, 0),IR(16960, 16)/* FlagIsProtected */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2,IR(0, 0),IR(16976, 14)/* FlagIsPrivate */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {3,IR(0, 0),IR(16990, 13)/* FlagIsPublic */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4,IR(0, 0),IR(17003, 12)/* FlagIsLocal */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8,IR(0, 0),IR(17015, 17)/* FlagIsDefinition */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16,IR(0, 0),IR(17032, 12)/* FlagFwdDecl */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32,IR(0, 0),IR(17044, 15)/* FlagArtificial */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {64,IR(0, 0),IR(17059, 13)/* FlagExplicit */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {128,IR(0, 0),IR(17072, 15)/* FlagPrototyped */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {256,IR(0, 0),IR(17087, 18)/* FlagObjectPointer */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {512,IR(0, 0),IR(17105, 17)/* FlagStaticMember */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {1024,IR(0, 0),IR(17122, 21)/* FlagIndirectVariable */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {2048,IR(0, 0),IR(17143, 20)/* FlagLValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {4096,IR(0, 0),IR(17163, 20)/* FlagRValueReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {8192,IR(0, 0),IR(17183, 16)/* FlagIsOptimized */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {16384,IR(0, 0),IR(17199, 16)/* FlagIsEnumClass */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {32768,IR(0, 0),IR(17215, 20)/* FlagTypePassByValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {65536,IR(0, 0),IR(17235, 24)/* FlagTypePassByReference */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {131072,IR(0, 0),IR(28676, 26)/* FlagUnknownPhysicalLayout */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {0,IR(0, 0),IR(17259, 6)/* Deref */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {1,IR(0, 0),IR(17265, 5)/* Plus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {2,IR(0, 0),IR(17270, 6)/* Minus */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {3,IR(10, 1),IR(17309, 11)/* PlusUconst */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {4,IR(36, 2),IR(17320, 9)/* BitPiece */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {5,IR(0, 0),IR(17329, 5)/* Swap */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {6,IR(0, 0),IR(17334, 7)/* Xderef */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {7,IR(0, 0),IR(17341, 11)/* StackValue */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {8,IR(10, 1),IR(17352, 7)/* Constu */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {9,IR(36, 2),IR(17359, 9)/* Fragment */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {0,IR(0, 0),IR(17368, 10)/* ConstType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {1,IR(0, 0),IR(17378, 13)/* VolatileType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {2,IR(0, 0),IR(17391, 13)/* RestrictType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {3,IR(0, 0),IR(17404, 11)/* AtomicType */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1, 0),0xffffffffu}, // SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {0,IR(0, 0),IR(28244, 8)/* Unknown */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {1,IR(0, 0),IR(28702, 5)/* ESSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {2,IR(0, 0),IR(28707, 5)/* GLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {3,IR(0, 0),IR(28712, 9)/* OpenCL_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {4,IR(0, 0),IR(28721, 11)/* OpenCL_CPP */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {5,IR(0, 0),IR(28732, 5)/* HLSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {6,IR(0, 0),IR(28737, 15)/* CPP_for_OpenCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {7,IR(0, 0),IR(28752, 5)/* SYCL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {8,IR(0, 0),IR(28757, 7)/* HERO_C */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {9,IR(0, 0),IR(28764, 5)/* NZSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {10,IR(0, 0),IR(28769, 5)/* WGSL */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {11,IR(0, 0),IR(28774, 6)/* Slang */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {12,IR(0, 0),IR(28780, 4)/* Zig */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {13,IR(0, 0),IR(28784, 5)/* Rust */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {14,IR(0, 0),IR(28789, 5)/* Pred */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {15,IR(0, 0),IR(28794, 9)/* ApilaJai */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {0,IR(0, 0),IR(28803, 16)/* UniformConstant */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {1,IR(0, 0),IR(28819, 6)/* Input */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {2,IR(0, 0),IR(18244, 8)/* Uniform */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {3,IR(0, 0),IR(28825, 7)/* Output */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4,IR(0, 0),IR(28556, 10)/* Workgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5,IR(0, 0),IR(28832, 15)/* CrossWorkgroup */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {6,IR(0, 0),IR(28847, 8)/* Private */,IR(0, 0),IR(215, 2),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {7,IR(0, 0),IR(28855, 9)/* Function */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {8,IR(0, 0),IR(28864, 8)/* Generic */,IR(0, 0),IR(217, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {9,IR(0, 0),IR(28872, 13)/* PushConstant */,IR(0, 0),IR(3, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {10,IR(0, 0),IR(28885, 14)/* AtomicCounter */,IR(0, 0),IR(84, 1),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {11,IR(0, 0),IR(28899, 6)/* Image */,IR(0, 0),IR(1, 0),IR(0, 0),SPV_SPIRV_VERSION_WORD(1,0),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {12,IR(0, 0),IR(28905, 14)/* StorageBuffer */,IR(0, 0),IR(3, 1),IR(213, 2),SPV_SPIRV_VERSION_WORD(1,3),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4172,IR(0, 0),IR(28919, 13)/* TileImageEXT */,IR(0, 0),IR(158, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {4491,IR(0, 0),IR(28932, 19)/* TileAttachmentQCOM */,IR(0, 0),IR(41, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5068,IR(0, 0),IR(28951, 16)/* NodePayloadAMDX */,IR(0, 0),IR(43, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5328,IR(0, 0),IR(28967, 16)/* CallableDataKHR */,IR(167, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5329,IR(0, 0),IR(28998, 24)/* IncomingCallableDataKHR */,IR(168, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5338,IR(0, 0),IR(29045, 14)/* RayPayloadKHR */,IR(169, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5339,IR(0, 0),IR(29072, 16)/* HitAttributeKHR */,IR(170, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5342,IR(0, 0),IR(29103, 22)/* IncomingRayPayloadKHR */,IR(171, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5343,IR(0, 0),IR(29146, 22)/* ShaderRecordBufferKHR */,IR(172, 1),IR(55, 2),IR(24, 2),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5349,IR(0, 0),IR(29189, 22)/* PhysicalStorageBuffer */,IR(173, 1),IR(2, 1),IR(0, 2),SPV_SPIRV_VERSION_WORD(1,5),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5385,IR(0, 0),IR(29236, 21)/* HitObjectAttributeNV */,IR(0, 0),IR(134, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5402,IR(0, 0),IR(29257, 24)/* TaskPayloadWorkgroupEXT */,IR(0, 0),IR(54, 1),IR(23, 1),SPV_SPIRV_VERSION_WORD(1,4),0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5411,IR(0, 0),IR(29281, 22)/* HitObjectAttributeEXT */,IR(0, 0),IR(135, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5605,IR(0, 0),IR(29303, 17)/* CodeSectionINTEL */,IR(0, 0),IR(218, 1),IR(115, 1),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5936,IR(0, 0),IR(29320, 17)/* DeviceOnlyALTERA */,IR(174, 1),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {5937,IR(0, 0),IR(29353, 15)/* HostOnlyALTERA */,IR(175, 1),IR(219, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORAGE_CLASS
  {0,IR(0, 0),IR(26164, 14)/* UncachedINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {1,IR(0, 0),IR(29382, 18)/* WriteThroughINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {2,IR(0, 0),IR(29400, 15)/* WriteBackINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {3,IR(0, 0),IR(26190, 15)/* StreamingINTEL */,IR(0, 0),IR(156, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {0,IR(0, 0),IR(16955, 5)/* None */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {1,IR(10, 1),IR(29415, 11)/* TensorView */,IR(0, 0),IR(220, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {2,IR(10, 1),IR(29426, 11)/* DecodeFunc */,IR(0, 0),IR(108, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {4,IR(10, 1),IR(29437, 17)/* DecodeVectorFunc */,IR(0, 0),IR(221, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {0,IR(0, 0),IR(29454, 10)/* Undefined */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {1,IR(0, 0),IR(18202, 9)/* Constant */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {2,IR(0, 0),IR(28189, 12)/* ClampToEdge */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {3,IR(0, 0),IR(28207, 7)/* Repeat */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {4,IR(0, 0),IR(28214, 15)/* RepeatMirrored */,IR(0, 0),IR(1, 0),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {0,IR(0, 0),IR(29464, 8)/* NoneARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {1,IR(0, 0),IR(29472, 15)/* NontemporalARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {2,IR(10, 1),IR(29487, 20)/* OutOfBoundsValueARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {4,IR(10, 1),IR(29507, 24)/* MakeElementAvailableARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {8,IR(10, 1),IR(29531, 22)/* MakeElementVisibleARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {16,IR(0, 0),IR(29553, 21)/* NonPrivateElementARM */,IR(0, 0),IR(222, 1),IR(0, 0),0xffffffffu,0xffffffffu}, // SPV_OPERAND_TYPE_TENSOR_OPERANDS
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
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(136, 306);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(442, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(450, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(453, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(455, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(474, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(484, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(488, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(503, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(507, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(513, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(516, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(519, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(523, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(531, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(534, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(550, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(559, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(562, 152);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(714, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(722, 101);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(823, 17);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(840, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(842, 8);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(850, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(852, 9);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(861, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(865, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(869, 6);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(875, 9);
    case SPV_OPERAND_TYPE_GATHER_MODES: return IR(884, 4);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(888, 7);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(895, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(899, 17);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(916, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(942, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(962, 2);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(964, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(967, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(969, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(971, 4);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(975, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(980, 21);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1001, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1016, 9);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1025, 4);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1029, 15);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1044, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1045, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1049, 1);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1050, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1058, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1061, 12);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1073, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1075, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1078, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1080, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1085, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1087, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1129, 7);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1136, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1139, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1140, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1148, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1151, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1153, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1173, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1183, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1187, 16);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1203, 29);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1232, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1236, 4);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1240, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1245, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(899, 17);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1016, 9);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1049, 1);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(507, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1001, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1058, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(842, 8);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1245, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(136, 306);
    default: break;
  }
  return IR(0,0);
}

// Operand names and index into kOperandsByValue, ordered by (operand kind, name)
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into the kOperandsByValue array
static const std::array<NameIndex, 1431> kOperandNames{{
  {IR(5931, 9), 0}, // 0 ReadOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5957, 10), 2}, // 1 ReadWrite in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5947, 10), 1}, // 2 WriteOnly in SPV_OPERAND_TYPE_ACCESS_QUALIFIER
  {IR(5967, 8), 3}, // 3 Logical in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5975, 11), 4}, // 4 Physical32 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(5996, 11), 5}, // 5 Physical64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6007, 24), 6}, // 6 PhysicalStorageBuffer64 in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(6031, 27), 6}, // 7 PhysicalStorageBuffer64EXT in SPV_OPERAND_TYPE_ADDRESSING_MODEL
  {IR(7990, 13), 94}, // 8 BaryCoordKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8003, 12), 94}, // 9 BaryCoordNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7336, 20), 68}, // 10 BaryCoordNoPerspAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7356, 28), 69}, // 11 BaryCoordNoPerspCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8038, 20), 95}, // 12 BaryCoordNoPerspKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8058, 19), 95}, // 13 BaryCoordNoPerspNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7384, 26), 70}, // 14 BaryCoordNoPerspSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7481, 22), 74}, // 15 BaryCoordPullModelAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7410, 19), 71}, // 16 BaryCoordSmoothAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7429, 27), 72}, // 17 BaryCoordSmoothCentroidAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7456, 25), 73}, // 18 BaryCoordSmoothSampleAMD in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7140, 13), 59}, // 19 BaseInstance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7114, 11), 58}, // 20 BaseVertex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6115, 13), 9}, // 21 ClipDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7897, 22), 89}, // 22 ClipDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9196, 12), 133}, // 23 ClusterIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6878, 13), 49}, // 24 CoreCountARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6852, 10), 48}, // 25 CoreIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6891, 13), 50}, // 26 CoreMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6128, 13), 10}, // 27 CullDistance in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7919, 22), 90}, // 28 CullDistancePerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9249, 12), 134}, // 29 CullMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8245, 17), 101}, // 30 CullPrimitiveEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8649, 17), 115}, // 31 CurrentRayTimeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9276, 24), 135}, // 32 DeviceBarrierValidINTEL in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7210, 12), 62}, // 33 DeviceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7153, 10), 60}, // 34 DrawIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6642, 22), 37}, // 35 EnqueuedWorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6412, 10), 21}, // 36 FragCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6498, 10), 27}, // 37 FragDepth in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8123, 23), 97}, // 38 FragInvocationCountEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8077, 12), 96}, // 39 FragSizeEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7503, 18), 75}, // 40 FragStencilRefEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8089, 15), 96}, // 41 FragmentSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6433, 12), 23}, // 42 FrontFacing in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7809, 16), 85}, // 43 FullyCoveredEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6583, 19), 33}, // 44 GlobalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6677, 15), 39}, // 45 GlobalLinearId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6664, 13), 38}, // 46 GlobalOffset in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6631, 11), 36}, // 47 GlobalSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6508, 17), 28}, // 48 HelperInvocation in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8951, 11), 122}, // 49 HitIsLSSNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8909, 14), 121}, // 50 HitIsSphereNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9131, 33), 130}, // 51 HitKindBackFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9097, 34), 129}, // 52 HitKindFrontFacingMicroTriangleNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8628, 11), 114}, // 53 HitKindKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8639, 10), 114}, // 54 HitKindNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9079, 18), 128}, // 55 HitLSSPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9182, 14), 132}, // 56 HitLSSRadiiNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8813, 37), 118}, // 57 HitMicroTriangleVertexBarycentricsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8746, 34), 117}, // 58 HitMicroTriangleVertexPositionsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9001, 20), 123}, // 59 HitSpherePositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9164, 18), 131}, // 60 HitSphereRadiusNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8621, 7), 113}, // 61 HitTNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8689, 30), 116}, // 62 HitTriangleVertexPositionsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8850, 20), 119}, // 63 IncomingRayFlagsKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8870, 19), 119}, // 64 IncomingRayFlagsNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8510, 23), 110}, // 65 InstanceCustomIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8533, 22), 110}, // 66 InstanceCustomIndexNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6150, 11), 12}, // 67 InstanceId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6838, 14), 47}, // 68 InstanceIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6251, 13), 14}, // 69 InvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8146, 22), 97}, // 70 InvocationsPerPixelNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8262, 12), 102}, // 71 LaunchIdKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8274, 11), 102}, // 72 LaunchIdNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8285, 14), 103}, // 73 LaunchSizeKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8299, 13), 103}, // 74 LaunchSizeNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6264, 6), 15}, // 75 Layer in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7941, 15), 91}, // 76 LayerPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6565, 18), 32}, // 77 LocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6602, 21), 34}, // 78 LocalInvocationIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7956, 16), 92}, // 79 MeshViewCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7972, 18), 93}, // 80 MeshViewIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6768, 21), 43}, // 81 NumEnqueuedSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6755, 13), 42}, // 82 NumSubgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6525, 14), 29}, // 83 NumWorkgroups in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8425, 22), 107}, // 84 ObjectRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8447, 21), 107}, // 85 ObjectRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8388, 19), 106}, // 86 ObjectRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8407, 18), 106}, // 87 ObjectRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8555, 17), 111}, // 88 ObjectToWorldKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8572, 16), 111}, // 89 ObjectToWorldNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6398, 14), 20}, // 90 PatchVertices in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6422, 11), 22}, // 91 PointCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6105, 10), 8}, // 92 PointSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6089, 9), 7}, // 93 Position in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7749, 18), 83}, // 94 PositionPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7861, 17), 87}, // 95 PrimitiveCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6161, 12), 13}, // 96 PrimitiveId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7878, 19), 88}, // 97 PrimitiveIndicesNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8193, 24), 99}, // 98 PrimitiveLineIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8168, 25), 98}, // 99 PrimitivePointIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7163, 24), 61}, // 100 PrimitiveShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8217, 28), 100}, // 101 PrimitiveTriangleIndicesEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8889, 20), 120}, // 102 RayGeometryIndexKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8489, 11), 109}, // 103 RayTmaxKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8500, 10), 109}, // 104 RayTmaxNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8468, 11), 108}, // 105 RayTminKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8479, 10), 108}, // 106 RayTminNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7538, 29), 76}, // 107 RemainingRecursionLevelsAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7634, 16), 79}, // 108 ResourceHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9053, 10), 125}, // 109 SMCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9072, 7), 127}, // 110 SMIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6445, 9), 24}, // 111 SampleId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6487, 11), 26}, // 112 SampleMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6472, 15), 25}, // 113 SamplePosition in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7601, 15), 78}, // 114 SamplerHeapEXT in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7686, 20), 81}, // 115 SecondaryPositionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7725, 24), 82}, // 116 SecondaryViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7585, 16), 77}, // 117 ShaderIndexAMDX in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7254, 15), 64}, // 118 ShadingRateKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6927, 15), 53}, // 119 SubgroupEqMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6942, 18), 53}, // 120 SubgroupEqMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6982, 15), 54}, // 121 SubgroupGeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6997, 18), 54}, // 122 SubgroupGeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7015, 15), 55}, // 123 SubgroupGtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7030, 18), 55}, // 124 SubgroupGtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6789, 11), 44}, // 125 SubgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7048, 15), 56}, // 126 SubgroupLeMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7063, 18), 56}, // 127 SubgroupLeMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6800, 26), 45}, // 128 SubgroupLocalInvocationId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7081, 15), 57}, // 129 SubgroupLtMask in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7096, 18), 57}, // 130 SubgroupLtMaskKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6739, 16), 41}, // 131 SubgroupMaxSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6692, 13), 40}, // 132 SubgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7849, 12), 86}, // 133 TaskCountNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6388, 10), 19}, // 134 TessCoord in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6373, 15), 18}, // 135 TessLevelInner in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6358, 15), 17}, // 136 TessLevelOuter in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7318, 18), 67}, // 137 TileApronSizeQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7300, 18), 66}, // 138 TileDimensionQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7269, 15), 65}, // 139 TileOffsetQCOM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6141, 9), 11}, // 140 VertexId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6826, 12), 46}, // 141 VertexIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7234, 10), 63}, // 142 ViewIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6310, 14), 16}, // 143 ViewportIndex in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7650, 15), 80}, // 144 ViewportMaskNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(7787, 22), 84}, // 145 ViewportMaskPerViewNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6904, 10), 51}, // 146 WarpIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9063, 9), 126}, // 147 WarpIDNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6914, 13), 52}, // 148 WarpMaxIDARM in SPV_OPERAND_TYPE_BUILT_IN
  {IR(9021, 13), 124}, // 149 WarpsPerSMNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6623, 8), 35}, // 150 WorkDim in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6553, 12), 31}, // 151 WorkgroupId in SPV_OPERAND_TYPE_BUILT_IN
  {IR(6539, 14), 30}, // 152 WorkgroupSize in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8347, 21), 105}, // 153 WorldRayDirectionKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8368, 20), 105}, // 154 WorldRayDirectionNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8312, 18), 104}, // 155 WorldRayOriginKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8330, 17), 104}, // 156 WorldRayOriginNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8588, 17), 112}, // 157 WorldToObjectKHR in SPV_OPERAND_TYPE_BUILT_IN
  {IR(8605, 16), 112}, // 158 WorldToObjectNV in SPV_OPERAND_TYPE_BUILT_IN
  {IR(11690, 9), 277}, // 159 AbortKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5986, 10), 140}, // 160 Addresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15134, 35), 384}, // 161 ArbitraryPrecisionFixedPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15169, 34), 384}, // 162 ArbitraryPrecisionFixedPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14606, 38), 372}, // 163 ArbitraryPrecisionFloatingPointALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14644, 37), 372}, // 164 ArbitraryPrecisionFloatingPointINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14541, 33), 371}, // 165 ArbitraryPrecisionIntegersALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14574, 32), 371}, // 166 ArbitraryPrecisionIntegersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15886, 19), 410}, // 167 ArithmeticFenceEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14179, 9), 357}, // 168 AsmINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15787, 20), 406}, // 169 AtomicFloat16AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14234, 23), 360}, // 170 AtomicFloat16MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13480, 22), 329}, // 171 AtomicFloat16VectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15703, 20), 402}, // 172 AtomicFloat32AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14188, 23), 358}, // 173 AtomicFloat32MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15723, 20), 403}, // 174 AtomicFloat64AddEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14211, 23), 359}, // 175 AtomicFloat64MinMaxEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9472, 14), 156}, // 176 AtomicStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10929, 17), 237}, // 177 AtomicStorageOps in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15828, 24), 408}, // 178 BFloat16ConversionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11661, 29), 276}, // 179 BFloat16CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11639, 22), 275}, // 180 BFloat16DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11353, 19), 258}, // 181 BFloat16MulAddQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11623, 16), 274}, // 182 BFloat16TypeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16662, 20), 436}, // 183 BindlessImagesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13417, 18), 326}, // 184 BindlessTextureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15598, 16), 397}, // 185 BitInstructions in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10580, 18), 221}, // 186 BitcastExtractEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15336, 20), 388}, // 187 BlockingPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15356, 19), 388}, // 188 BlockingPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16623, 19), 434}, // 189 CacheControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6115, 13), 166}, // 190 ClipDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13019, 32), 314}, // 191 ComputeDerivativeGroupLinearKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13051, 31), 314}, // 192 ComputeDerivativeGroupLinearNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11869, 31), 293}, // 193 ComputeDerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11900, 30), 293}, // 194 ComputeDerivativeGroupQuadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11699, 16), 279}, // 195 ConstantDataKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13818, 30), 342}, // 196 CooperativeMatrixBlockLoadsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11298, 32), 256}, // 197 CooperativeMatrixConversionQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13577, 32), 337}, // 198 CooperativeMatrixConversionsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13670, 31), 339}, // 199 CooperativeMatrixConversionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13929, 32), 347}, // 200 CooperativeMatrixDecodeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13876, 34), 345}, // 201 CooperativeMatrixGetCoordinateEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10507, 21), 395}, // 202 CooperativeMatrixKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10442, 28), 214}, // 203 CooperativeMatrixLayoutsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13107, 20), 316}, // 204 CooperativeMatrixNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13701, 41), 340}, // 205 CooperativeMatrixPerElementOperationsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13742, 40), 340}, // 206 CooperativeMatrixPerElementOperationsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13609, 31), 338}, // 207 CooperativeMatrixReductionsEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13640, 30), 338}, // 208 CooperativeMatrixReductionsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13782, 36), 341}, // 209 CooperativeMatrixTensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13460, 20), 328}, // 210 CooperativeVectorNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13848, 28), 343}, // 211 CooperativeVectorTrainingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6862, 16), 206}, // 212 CoreBuiltinsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6128, 13), 167}, // 213 CullDistance in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15807, 21), 407}, // 214 DebugInfoModuleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13230, 25), 321}, // 215 DemoteToHelperInvocation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13255, 28), 321}, // 216 DemoteToHelperInvocationEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11068, 18), 243}, // 217 DenormFlushToZero in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11053, 15), 242}, // 218 DenormPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9919, 18), 185}, // 219 DerivativeControl in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7616, 18), 278}, // 220 DescriptorHeapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9300, 19), 417}, // 221 DeviceBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9443, 14), 154}, // 222 DeviceEnqueue in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7222, 12), 233}, // 223 DeviceGroup in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13283, 23), 322}, // 224 DisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15549, 11), 393}, // 225 DotProduct in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16748, 27), 439}, // 226 DotProductBFloat16AccVALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16715, 33), 438}, // 227 DotProductFloat16AccFloat16VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16682, 33), 437}, // 228 DotProductFloat16AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16775, 32), 440}, // 229 DotProductFloat8AccFloat32VALVE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15443, 22), 391}, // 230 DotProductInput4x8Bit in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15465, 25), 391}, // 231 DotProductInput4x8BitKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15490, 28), 392}, // 232 DotProductInput4x8BitPacked in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15518, 31), 392}, // 233 DotProductInput4x8BitPackedKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15402, 19), 390}, // 234 DotProductInputAll in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15421, 22), 390}, // 235 DotProductInputAllKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15560, 14), 393}, // 236 DotProductKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7125, 15), 224}, // 237 DrawParameters in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14291, 16), 363}, // 238 ExpectAssumeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15654, 7), 400}, // 239 FMAKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14521, 20), 370}, // 240 FPFastMathModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16094, 29), 416}, // 241 FPGAArgumentInterfacesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16123, 28), 416}, // 242 FPGAArgumentInterfacesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15085, 25), 383}, // 243 FPGABufferLocationALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15110, 24), 383}, // 244 FPGABufferLocationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14853, 28), 378}, // 245 FPGAClusterAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14881, 27), 378}, // 246 FPGAClusterAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15905, 30), 411}, // 247 FPGAClusterAttributesV2ALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15935, 29), 411}, // 248 FPGAClusterAttributesV2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14937, 21), 380}, // 249 FPGADSPControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14958, 20), 380}, // 250 FPGADSPControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15004, 41), 382}, // 251 FPGAInvocationPipeliningAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15045, 40), 382}, // 252 FPGAInvocationPipeliningAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14778, 26), 376}, // 253 FPGAKernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15964, 28), 412}, // 254 FPGAKernelAttributesv2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16045, 25), 415}, // 255 FPGALatencyControlALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16070, 24), 415}, // 256 FPGALatencyControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14711, 23), 374}, // 257 FPGALoopControlsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14734, 22), 374}, // 258 FPGALoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14804, 25), 377}, // 259 FPGAMemoryAccessesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14829, 24), 377}, // 260 FPGAMemoryAccessesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14468, 27), 369}, // 261 FPGAMemoryAttributesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14495, 26), 369}, // 262 FPGAMemoryAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15375, 14), 389}, // 263 FPGARegALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15389, 13), 389}, // 264 FPGARegINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16029, 16), 414}, // 265 FPMaxErrorINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9357, 8), 145}, // 266 Float16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9343, 14), 144}, // 267 Float16Buffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11463, 16), 263}, // 268 Float16ImageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10538, 10), 218}, // 269 Float4EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9365, 8), 146}, // 270 Float64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10528, 10), 217}, // 271 Float6EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10480, 27), 216}, // 272 Float8CooperativeMatrixEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10470, 10), 215}, // 273 Float8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10548, 22), 219}, // 274 Float8UnsignedE8M0EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15639, 15), 399}, // 275 FloatControls2 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14087, 23), 353}, // 276 FloatingPointModeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8015, 23), 292}, // 277 FragmentBarycentricKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11847, 22), 292}, // 278 FragmentBarycentricNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8104, 19), 294}, // 279 FragmentDensityEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7825, 24), 288}, // 280 FragmentFullyCoveredEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11501, 16), 265}, // 281 FragmentMaskAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13198, 32), 320}, // 282 FragmentShaderPixelInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13127, 33), 317}, // 283 FragmentShaderSampleInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13160, 38), 318}, // 284 FragmentShaderShadingRateInterlockEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7187, 23), 222}, // 285 FragmentShadingRateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14442, 26), 368}, // 286 FunctionFloatControlINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14133, 22), 355}, // 287 FunctionPointersINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16482, 22), 428}, // 288 FunctionVariantsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9764, 15), 172}, // 289 GenericPointer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6173, 9), 138}, // 290 Geometry in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9514, 18), 159}, // 291 GeometryPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11775, 28), 283}, // 292 GeometryShaderPassthroughNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9977, 16), 188}, // 293 GeometryStreams in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16181, 36), 419}, // 294 GlobalVariableFPGADecorationsALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16217, 35), 419}, // 295 GlobalVariableFPGADecorationsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16151, 30), 418}, // 296 GlobalVariableHostAccessINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10433, 9), 213}, // 297 GraphARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6705, 16), 195}, // 298 GroupNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10116, 26), 197}, // 299 GroupNonUniformArithmetic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6960, 22), 198}, // 300 GroupNonUniformBallot in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10196, 25), 201}, // 301 GroupNonUniformClustered in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11944, 30), 295}, // 302 GroupNonUniformPartitionedEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11974, 29), 295}, // 303 GroupNonUniformPartitionedNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10221, 20), 202}, // 304 GroupNonUniformQuad in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15614, 25), 398}, // 305 GroupNonUniformRotateKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10142, 23), 199}, // 306 GroupNonUniformShuffle in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10165, 31), 200}, // 307 GroupNonUniformShuffleRelative in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10096, 20), 196}, // 308 GroupNonUniformVote in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16545, 26), 431}, // 309 GroupUniformArithmeticKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9436, 7), 153}, // 310 Groups in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15309, 14), 387}, // 311 IOPipesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15323, 13), 387}, // 312 IOPipesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9833, 8), 178}, // 313 Image1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9392, 11), 149}, // 314 ImageBasic in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9855, 12), 181}, // 315 ImageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9710, 15), 168}, // 316 ImageCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11830, 17), 290}, // 317 ImageFootprintNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11479, 22), 264}, // 318 ImageGatherBiasLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9532, 20), 160}, // 319 ImageGatherExtended in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11434, 29), 262}, // 320 ImageGatherExtendedModesQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11412, 22), 261}, // 321 ImageGatherLinearQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9867, 13), 182}, // 322 ImageMSArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9418, 12), 151}, // 323 ImageMipmap in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9908, 11), 184}, // 324 ImageQuery in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9403, 15), 150}, // 325 ImageReadWrite in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11517, 21), 267}, // 326 ImageReadWriteLodAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9742, 10), 170}, // 327 ImageRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14155, 24), 356}, // 328 IndirectReferencesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9784, 16), 174}, // 329 InputAttachment in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12089, 36), 298}, // 330 InputAttachmentArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12125, 39), 298}, // 331 InputAttachmentArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12630, 39), 305}, // 332 InputAttachmentArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12669, 42), 305}, // 333 InputAttachmentArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9486, 6), 157}, // 334 Int16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11596, 27), 273}, // 335 Int4CooperativeMatrixINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11582, 14), 272}, // 336 Int4TypeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9373, 6), 147}, // 337 Int64 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9379, 13), 148}, // 338 Int64Atomics in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11538, 14), 268}, // 339 Int64ImageEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9779, 5), 173}, // 340 Int8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14110, 23), 354}, // 341 IntegerFunctions2INTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9937, 22), 186}, // 342 InterpolationFunction in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16807, 17), 441}, // 343 IntrinsicSAMSUNG in SPV_OPERAND_TYPE_CAPABILITY
  {IR(5940, 7), 142}, // 344 Kernel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14756, 22), 375}, // 345 KernelAttributesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9326, 8), 141}, // 346 Linkage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9457, 15), 155}, // 347 LiteralSampler in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15743, 20), 404}, // 348 LongCompositesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13540, 14), 335}, // 349 LongVectorEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14908, 15), 379}, // 350 LoopFuseALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14923, 14), 379}, // 351 LoopFuseINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10570, 10), 220}, // 352 MXInt8EXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16598, 25), 433}, // 353 MaskedGatherScatterINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9319, 7), 136}, // 354 Matrix in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14978, 26), 381}, // 355 MemoryAccessAliasingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6236, 15), 291}, // 356 MeshShadingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6222, 14), 289}, // 357 MeshShadingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9816, 7), 176}, // 358 MinLod in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7244, 10), 234}, // 359 MultiView in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6324, 14), 191}, // 360 MultiViewport in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11389, 23), 260}, // 361 MultipleWaitQueuesQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10071, 13), 193}, // 362 NamedBarrier in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15763, 11), 405}, // 363 OptNoneEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15774, 13), 405}, // 364 OptNoneINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7767, 20), 287}, // 365 PerViewAttributesNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6058, 31), 313}, // 366 PhysicalStorageBufferAddresses in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12985, 34), 313}, // 367 PhysicalStorageBufferAddressesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10084, 12), 194}, // 368 PipeStorage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9430, 6), 152}, // 369 Pipes in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11715, 16), 280}, // 370 PoisonFreezeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16504, 18), 429}, // 371 PredicatedIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13520, 20), 334}, // 372 PushConstantBanksNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11567, 15), 271}, // 373 QuadControlKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13502, 18), 331}, // 374 RawAccessChainsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9261, 15), 394}, // 375 RayCullMaskKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11166, 12), 248}, // 376 RayQueryKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13435, 25), 327}, // 377 RayQueryPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11143, 23), 247}, // 378 RayQueryProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9208, 41), 344}, // 379 RayTracingClusterAccelerationStructureNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8780, 33), 330}, // 380 RayTracingDisplacementMicromapNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6208, 14), 251}, // 381 RayTracingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8962, 39), 333}, // 382 RayTracingLinearSweptSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8666, 23), 310}, // 383 RayTracingMotionBlurNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6195, 13), 309}, // 384 RayTracingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13335, 29), 323}, // 385 RayTracingOpacityMicromapEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15661, 42), 401}, // 386 RayTracingOpacityMicromapExecutionModeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13306, 29), 323}, // 387 RayTracingOpacityMicromapKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8719, 27), 308}, // 388 RayTracingPositionFetchKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13082, 25), 315}, // 389 RayTracingProvisionalKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(8923, 28), 332}, // 390 RayTracingSpheresGeometryNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11197, 32), 250}, // 391 RayTraversalPrimitiveCullingKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16642, 20), 435}, // 392 RegisterLimitsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15574, 24), 396}, // 393 ReplicatedCompositesEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14066, 21), 352}, // 394 RoundToInfinityINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16522, 23), 430}, // 395 RoundedDivideSqrtINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11111, 16), 245}, // 396 RoundingModeRTE in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11127, 16), 246}, // 397 RoundingModeRTZ in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15250, 30), 386}, // 398 RuntimeAlignedAttributeALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15280, 29), 386}, // 399 RuntimeAlignedAttributeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12040, 23), 297}, // 400 RuntimeDescriptorArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12063, 26), 297}, // 401 RuntimeDescriptorArrayEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11746, 29), 282}, // 402 SampleMaskOverrideCoverageNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10946, 28), 238}, // 403 SampleMaskPostDepthCoverage in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6454, 18), 169}, // 404 SampleRateShading in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9823, 10), 177}, // 405 Sampled1D in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9841, 14), 180}, // 406 SampledBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9725, 17), 179}, // 407 SampledCubeArray in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9610, 33), 163}, // 408 SampledImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12403, 36), 302}, // 409 SampledImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12439, 39), 302}, // 410 SampledImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9752, 12), 171}, // 411 SampledRect in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6098, 7), 137}, // 412 Shader in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13554, 23), 336}, // 413 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11552, 15), 269}, // 414 ShaderClockKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7567, 18), 270}, // 415 ShaderEnqueueAMDX in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13390, 27), 325}, // 416 ShaderInvocationReorderEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13364, 26), 324}, // 417 ShaderInvocationReorderNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6270, 12), 203}, // 418 ShaderLayer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12003, 17), 296}, // 419 ShaderNonUniform in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12020, 20), 296}, // 420 ShaderNonUniformEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9034, 19), 319}, // 421 ShaderSMBuiltinsNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7706, 19), 286}, // 422 ShaderStereoViewNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6338, 20), 204}, // 423 ShaderViewportIndex in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6282, 28), 284}, // 424 ShaderViewportIndexLayerEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11803, 27), 284}, // 425 ShaderViewportIndexLayerNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7665, 21), 285}, // 426 ShaderViewportMaskNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11930, 14), 294}, // 427 ShadingRateNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11086, 25), 244}, // 428 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9800, 16), 175}, // 429 SparseResidency in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16461, 21), 427}, // 430 SpecConditionalINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15852, 16), 409}, // 431 SplitBarrierEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15868, 18), 409}, // 432 SplitBarrierINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7521, 17), 266}, // 433 StencilExportEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10734, 25), 229}, // 434 StorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10974, 24), 239}, // 435 StorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9643, 34), 164}, // 436 StorageBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12478, 37), 303}, // 437 StorageBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12515, 40), 303}, // 438 StorageBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9677, 33), 165}, // 439 StorageImageArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12555, 36), 304}, // 440 StorageImageArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12591, 39), 304}, // 441 StorageImageArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9880, 28), 183}, // 442 StorageImageExtendedFormats in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9552, 24), 161}, // 443 StorageImageMultisample in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9993, 30), 189}, // 444 StorageImageReadWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10023, 31), 190}, // 445 StorageImageWriteWithoutFormat in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10861, 21), 232}, // 446 StorageInputOutput16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10839, 22), 231}, // 447 StoragePushConstant16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11032, 21), 241}, // 448 StoragePushConstant8 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10356, 37), 211}, // 449 StorageTensorArrayDynamicIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10393, 40), 212}, // 450 StorageTensorArrayNonUniformIndexingARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12245, 39), 300}, // 451 StorageTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12284, 42), 300}, // 452 StorageTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12798, 42), 307}, // 453 StorageTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12840, 45), 307}, // 454 StorageTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10822, 17), 230}, // 455 StorageUniform16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10759, 28), 229}, // 456 StorageUniformBufferBlock16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16280, 23), 421}, // 457 Subgroup2DBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16303, 30), 422}, // 458 Subgroup2DBlockTransformINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16333, 30), 423}, // 459 Subgroup2DBlockTransposeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14378, 39), 366}, // 460 SubgroupAvcMotionEstimationChromaINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14307, 33), 364}, // 461 SubgroupAvcMotionEstimationINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14340, 38), 365}, // 462 SubgroupAvcMotionEstimationIntraINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6721, 18), 223}, // 463 SubgroupBallotKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13982, 27), 349}, // 464 SubgroupBufferBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16252, 28), 420}, // 465 SubgroupBufferPrefetchINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10054, 17), 192}, // 466 SubgroupDispatch in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14009, 26), 350}, // 467 SubgroupImageBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14035, 31), 351}, // 468 SubgroupImageMediaBlockIOINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16363, 38), 424}, // 469 SubgroupMatrixMultiplyAccumulateINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13961, 21), 348}, // 470 SubgroupShuffleINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11372, 17), 259}, // 471 SubgroupSizeQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10718, 16), 228}, // 472 SubgroupVoteKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15992, 19), 413}, // 473 TaskSequenceALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16011, 18), 413}, // 474 TaskSequenceINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(13910, 19), 346}, // 475 TensorAddressingNV in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16571, 27), 432}, // 476 TensorFloat32RoundingINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10345, 11), 210}, // 477 TensorsARM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16401, 28), 425}, // 478 TernaryBitwiseFunctionINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(6182, 13), 139}, // 479 Tessellation in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9492, 22), 158}, // 480 TessellationPointSize in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11330, 23), 257}, // 481 TextureBlockMatch2QCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11276, 22), 254}, // 482 TextureBlockMatchQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11255, 21), 253}, // 483 TextureBoxFilterQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11229, 26), 252}, // 484 TextureSampleWeightedQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10259, 28), 207}, // 485 TileImageColorReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10287, 28), 208}, // 486 TileImageDepthReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10315, 30), 209}, // 487 TileImageStencilReadAccessEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(7284, 16), 255}, // 488 TileShadingQCOM in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9959, 18), 187}, // 489 TransformFeedback in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15203, 24), 385}, // 490 USMStorageClassesALTERA in SPV_OPERAND_TYPE_CAPABILITY
  {IR(15227, 23), 385}, // 491 USMStorageClassesINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10787, 35), 230}, // 492 UniformAndStorageBuffer16BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10998, 34), 240}, // 493 UniformAndStorageBuffer8BitAccess in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9576, 34), 162}, // 494 UniformBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12326, 37), 301}, // 495 UniformBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12363, 40), 301}, // 496 UniformBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10241, 18), 205}, // 497 UniformDecoration in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12164, 39), 299}, // 498 UniformTexelBufferArrayDynamicIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12203, 42), 299}, // 499 UniformTexelBufferArrayDynamicIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12711, 42), 306}, // 500 UniformTexelBufferArrayNonUniformIndexing in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12753, 45), 306}, // 501 UniformTexelBufferArrayNonUniformIndexingEXT in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14681, 30), 373}, // 502 UnstructuredLoopControlsINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11178, 19), 249}, // 503 UntypedPointersKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16429, 32), 426}, // 504 UntypedVariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14417, 25), 367}, // 505 VariableLengthArrayINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10912, 17), 236}, // 506 VariablePointers in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10882, 30), 235}, // 507 VariablePointersStorageBuffer in SPV_OPERAND_TYPE_CAPABILITY
  {IR(9334, 9), 143}, // 508 Vector16 in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14276, 15), 362}, // 509 VectorAnyINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(14257, 19), 361}, // 510 VectorComputeINTEL in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12885, 18), 311}, // 511 VulkanMemoryModel in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12924, 29), 312}, // 512 VulkanMemoryModelDeviceScope in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12953, 32), 312}, // 513 VulkanMemoryModelDeviceScopeKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(12903, 21), 311}, // 514 VulkanMemoryModelKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(11731, 15), 281}, // 515 WeakLinkageAMD in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10674, 44), 227}, // 516 WorkgroupMemoryExplicitLayout16BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10631, 43), 226}, // 517 WorkgroupMemoryExplicitLayout8BitAccessKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(10598, 33), 225}, // 518 WorkgroupMemoryExplicitLayoutKHR in SPV_OPERAND_TYPE_CAPABILITY
  {IR(16836, 8), 443}, // 519 Address in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16844, 8), 444}, // 520 Boolean in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16852, 6), 445}, // 521 Float in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16858, 7), 446}, // 522 Signed in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16865, 11), 447}, // 523 SignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16876, 9), 448}, // 524 Unsigned in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16885, 13), 449}, // 525 UnsignedChar in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16824, 12), 442}, // 526 Unspecified in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16898, 6), 450}, // 527 Class in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16904, 10), 451}, // 528 Structure in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16914, 6), 452}, // 529 Union in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16935, 20), 454}, // 530 ImportedDeclaration in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16920, 15), 453}, // 531 ImportedModule in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(17044, 15), 462}, // 532 FlagArtificial in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17059, 13), 463}, // 533 FlagExplicit in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17032, 12), 461}, // 534 FlagFwdDecl in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17122, 21), 467}, // 535 FlagIndirectVariable in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17015, 17), 460}, // 536 FlagIsDefinition in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17199, 16), 471}, // 537 FlagIsEnumClass in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17003, 12), 459}, // 538 FlagIsLocal in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17183, 16), 470}, // 539 FlagIsOptimized in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16976, 14), 457}, // 540 FlagIsPrivate in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16960, 16), 456}, // 541 FlagIsProtected in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16990, 13), 458}, // 542 FlagIsPublic in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17143, 20), 468}, // 543 FlagLValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17087, 18), 465}, // 544 FlagObjectPointer in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17072, 15), 464}, // 545 FlagPrototyped in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17163, 20), 469}, // 546 FlagRValueReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17105, 17), 466}, // 547 FlagStaticMember in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17235, 24), 473}, // 548 FlagTypePassByReference in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17215, 20), 472}, // 549 FlagTypePassByValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(16955, 5), 455}, // 550 None in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS
  {IR(17320, 9), 478}, // 551 BitPiece in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17352, 7), 482}, // 552 Constu in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17259, 6), 474}, // 553 Deref in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17359, 9), 483}, // 554 Fragment in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17270, 6), 476}, // 555 Minus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17265, 5), 475}, // 556 Plus in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17309, 11), 477}, // 557 PlusUconst in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17341, 11), 481}, // 558 StackValue in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17329, 5), 479}, // 559 Swap in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17334, 7), 480}, // 560 Xderef in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION
  {IR(17404, 11), 487}, // 561 AtomicType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17368, 10), 484}, // 562 ConstType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17391, 13), 486}, // 563 RestrictType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17378, 13), 485}, // 564 VolatileType in SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17415, 10), 488}, // 565 Float16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17425, 10), 489}, // 566 Float32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17435, 10), 490}, // 567 Float64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17603, 12), 501}, // 568 FloatE4M3NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17615, 12), 502}, // 569 FloatE5M2NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17458, 14), 492}, // 570 SignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17472, 14), 493}, // 571 SignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17486, 14), 494}, // 572 SignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17445, 13), 491}, // 573 SignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17563, 19), 499}, // 574 SignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17515, 16), 496}, // 575 UnsignedInt16NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17531, 16), 497}, // 576 UnsignedInt32NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17547, 16), 498}, // 577 UnsignedInt64NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17500, 15), 495}, // 578 UnsignedInt8NV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17582, 21), 500}, // 579 UnsignedInt8PackedNV in SPV_OPERAND_TYPE_COMPONENT_TYPE
  {IR(17679, 28), 506}, // 580 ColumnBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17639, 15), 504}, // 581 ColumnMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17654, 25), 505}, // 582 RowBlockedInterleavedARM in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17627, 12), 503}, // 583 RowMajorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT
  {IR(17715, 27), 508}, // 584 MatrixASignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17742, 27), 509}, // 585 MatrixBSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17769, 27), 510}, // 586 MatrixCSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17796, 32), 511}, // 587 MatrixResultSignedComponentsKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17707, 8), 507}, // 588 NoneKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17828, 26), 512}, // 589 SaturatingAccumulationKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS
  {IR(17865, 4), 515}, // 590 2x2 in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17858, 7), 514}, // 591 Column in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17854, 4), 513}, // 592 Row in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE
  {IR(17869, 11), 516}, // 593 MatrixAKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17891, 21), 518}, // 594 MatrixAccumulatorKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17880, 11), 517}, // 595 MatrixBKHR in SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE
  {IR(17923, 14), 520}, // 596 ColumnMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17937, 21), 521}, // 597 InferencingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17912, 11), 519}, // 598 RowMajorNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(17958, 18), 522}, // 599 TrainingOptimalNV in SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT
  {IR(16836, 8), 524}, // 600 Address in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16844, 8), 525}, // 601 Boolean in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16852, 6), 526}, // 602 Float in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16858, 7), 527}, // 603 Signed in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16865, 11), 528}, // 604 SignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16876, 9), 529}, // 605 Unsigned in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16885, 13), 530}, // 606 UnsignedChar in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16824, 12), 523}, // 607 Unspecified in SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16898, 6), 531}, // 608 Class in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16904, 10), 532}, // 609 Structure in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(16914, 6), 533}, // 610 Union in SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE
  {IR(17044, 15), 541}, // 611 FlagArtificial in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17059, 13), 542}, // 612 FlagExplicit in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17032, 12), 540}, // 613 FlagFwdDecl in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17122, 21), 546}, // 614 FlagIndirectVariable in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17015, 17), 539}, // 615 FlagIsDefinition in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17003, 12), 538}, // 616 FlagIsLocal in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17183, 16), 549}, // 617 FlagIsOptimized in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16976, 14), 536}, // 618 FlagIsPrivate in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16960, 16), 535}, // 619 FlagIsProtected in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16990, 13), 537}, // 620 FlagIsPublic in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17143, 20), 547}, // 621 FlagLValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17087, 18), 544}, // 622 FlagObjectPointer in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17072, 15), 543}, // 623 FlagPrototyped in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17163, 20), 548}, // 624 FlagRValueReference in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17105, 17), 545}, // 625 FlagStaticMember in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(16955, 5), 534}, // 626 None in SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS
  {IR(17320, 9), 554}, // 627 BitPiece in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17352, 7), 558}, // 628 Constu in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17259, 6), 550}, // 629 Deref in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17270, 6), 552}, // 630 Minus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17265, 5), 551}, // 631 Plus in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17309, 11), 553}, // 632 PlusUconst in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17341, 11), 557}, // 633 StackValue in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17329, 5), 555}, // 634 Swap in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17334, 7), 556}, // 635 Xderef in SPV_OPERAND_TYPE_DEBUG_OPERATION
  {IR(17368, 10), 559}, // 636 ConstType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17391, 13), 561}, // 637 RestrictType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(17378, 13), 560}, // 638 VolatileType in SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER
  {IR(20726, 16), 683}, // 639 AliasScopeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18185, 8), 581}, // 640 Aliased in SPV_OPERAND_TYPE_DECORATION
  {IR(19346, 15), 637}, // 641 AliasedPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(19361, 18), 637}, // 642 AliasedPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18663, 10), 605}, // 643 Alignment in SPV_OPERAND_TYPE_DECORATION
  {IR(18707, 12), 607}, // 644 AlignmentId in SPV_OPERAND_TYPE_DECORATION
  {IR(18036, 12), 568}, // 645 ArrayStride in SPV_OPERAND_TYPE_DECORATION
  {IR(19087, 17), 624}, // 646 ArrayStrideIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(20276, 15), 671}, // 647 BankBitsALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20291, 14), 671}, // 648 BankBitsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19455, 7), 641}, // 649 BankNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19989, 16), 664}, // 650 BankwidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20005, 15), 664}, // 651 BankwidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18340, 8), 594}, // 652 Binding in SPV_OPERAND_TYPE_DECORATION
  {IR(19480, 16), 643}, // 653 BindlessImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19462, 18), 642}, // 654 BindlessSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18000, 6), 564}, // 655 Block in SPV_OPERAND_TYPE_DECORATION
  {IR(18847, 22), 614}, // 656 BlockMatchSamplerQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(18825, 22), 613}, // 657 BlockMatchTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(19511, 13), 645}, // 658 BoundImageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19496, 15), 644}, // 659 BoundSamplerNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18006, 12), 565}, // 660 BufferBlock in SPV_OPERAND_TYPE_DECORATION
  {IR(20886, 21), 688}, // 661 BufferLocationALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20907, 20), 688}, // 662 BufferLocationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18117, 8), 573}, // 663 BuiltIn in SPV_OPERAND_TYPE_DECORATION
  {IR(20445, 20), 676}, // 664 BurstCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20465, 19), 676}, // 665 BurstCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18083, 8), 572}, // 666 CPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(22133, 22), 711}, // 667 CacheControlLoadINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(22192, 23), 712}, // 668 CacheControlStoreINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20484, 16), 677}, // 669 CacheSizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20500, 15), 677}, // 670 CacheSizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18150, 9), 577}, // 671 Centroid in SPV_OPERAND_TYPE_DECORATION
  {IR(19594, 13), 649}, // 672 ClobberINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18211, 9), 584}, // 673 Coherent in SPV_OPERAND_TYPE_DECORATION
  {IR(18027, 9), 567}, // 674 ColMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(18324, 10), 592}, // 675 Component in SPV_OPERAND_TYPE_DECORATION
  {IR(22080, 17), 710}, // 676 ConditionalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21309, 28), 698}, // 677 ConduitKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21337, 27), 698}, // 678 ConduitKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18202, 9), 583}, // 679 Constant in SPV_OPERAND_TYPE_DECORATION
  {IR(19524, 30), 646}, // 680 CooperativeMatrixTransposeEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19740, 14), 656}, // 681 CounterBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(18348, 14), 595}, // 682 DescriptorSet in SPV_OPERAND_TYPE_DECORATION
  {IR(20515, 29), 678}, // 683 DontStaticallyCoalesceALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20544, 28), 678}, // 684 DontStaticallyCoalesceINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20098, 17), 667}, // 685 DoublepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20115, 16), 667}, // 686 DoublepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18869, 18), 615}, // 687 ExplicitInterpAMD in SPV_OPERAND_TYPE_DECORATION
  {IR(18533, 15), 601}, // 688 FPFastMathMode in SPV_OPERAND_TYPE_DECORATION
  {IR(21171, 26), 695}, // 689 FPMaxErrorDecorationINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18483, 15), 600}, // 690 FPRoundingMode in SPV_OPERAND_TYPE_DECORATION
  {IR(18139, 5), 575}, // 691 Flat in SPV_OPERAND_TYPE_DECORATION
  {IR(20305, 21), 672}, // 692 ForcePow2DepthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20326, 20), 672}, // 693 ForcePow2DepthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18435, 14), 599}, // 694 FuncParamAttr in SPV_OPERAND_TYPE_DECORATION
  {IR(19651, 21), 652}, // 695 FuncParamIOKindINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19882, 24), 660}, // 696 FunctionDenormModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21000, 31), 690}, // 697 FunctionFloatingPointModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19825, 26), 659}, // 698 FunctionRoundingModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20636, 26), 681}, // 699 FuseLoopsInFunctionALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20662, 25), 681}, // 700 FuseLoopsInFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18072, 11), 571}, // 701 GLSLPacked in SPV_OPERAND_TYPE_DECORATION
  {IR(18061, 11), 570}, // 702 GLSLShared in SPV_OPERAND_TYPE_DECORATION
  {IR(19714, 26), 655}, // 703 GlobalVariableOffsetINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19424, 31), 640}, // 704 HitObjectShaderRecordBufferEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19394, 30), 639}, // 705 HitObjectShaderRecordBufferNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19754, 24), 656}, // 706 HlslCounterBufferGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(19791, 19), 657}, // 707 HlslSemanticGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(21931, 16), 707}, // 708 HostAccessINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20927, 20), 689}, // 709 IOPipeStorageALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20947, 19), 689}, // 710 IOPipeStorageINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(22023, 29), 709}, // 711 ImplementInRegisterMapALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(22052, 28), 709}, // 712 ImplementInRegisterMapINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18334, 6), 593}, // 713 Index in SPV_OPERAND_TYPE_DECORATION
  {IR(21994, 15), 708}, // 714 InitModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(22009, 14), 708}, // 715 InitModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20755, 25), 685}, // 716 InitiationIntervalALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20780, 24), 685}, // 717 InitiationIntervalINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18642, 21), 604}, // 718 InputAttachmentIndex in SPV_OPERAND_TYPE_DECORATION
  {IR(16807, 17), 713}, // 719 IntrinsicSAMSUNG in SPV_OPERAND_TYPE_DECORATION
  {IR(18166, 10), 579}, // 720 Invariant in SPV_OPERAND_TYPE_DECORATION
  {IR(21248, 31), 697}, // 721 LatencyControlConstraintALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21279, 30), 697}, // 722 LatencyControlConstraintINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21197, 26), 696}, // 723 LatencyControlLabelALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21223, 25), 696}, // 724 LatencyControlLabelINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18610, 18), 602}, // 725 LinkageAttributes in SPV_OPERAND_TYPE_DECORATION
  {IR(18315, 9), 591}, // 726 Location in SPV_OPERAND_TYPE_DECORATION
  {IR(21427, 34), 700}, // 727 MMHostInterfaceAddressWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21461, 33), 700}, // 728 MMHostInterfaceAddressWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21494, 31), 701}, // 729 MMHostInterfaceDataWidthALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21525, 30), 701}, // 730 MMHostInterfaceDataWidthINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21555, 29), 702}, // 731 MMHostInterfaceLatencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21584, 28), 702}, // 732 MMHostInterfaceLatencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21715, 30), 704}, // 733 MMHostInterfaceMaxBurstALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21745, 29), 704}, // 734 MMHostInterfaceMaxBurstINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21646, 35), 703}, // 735 MMHostInterfaceReadWriteModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21681, 34), 703}, // 736 MMHostInterfaceReadWriteModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21774, 33), 705}, // 737 MMHostInterfaceWaitRequestALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21807, 32), 705}, // 738 MMHostInterfaceWaitRequestINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20687, 20), 682}, // 739 MathOpDSPModeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20707, 19), 682}, // 740 MathOpDSPModeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18048, 13), 569}, // 741 MatrixStride in SPV_OPERAND_TYPE_DECORATION
  {IR(18673, 14), 606}, // 742 MaxByteOffset in SPV_OPERAND_TYPE_DECORATION
  {IR(18719, 16), 608}, // 743 MaxByteOffsetId in SPV_OPERAND_TYPE_DECORATION
  {IR(20804, 21), 686}, // 744 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20825, 20), 686}, // 745 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20020, 23), 665}, // 746 MaxPrivateCopiesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20043, 22), 665}, // 747 MaxPrivateCopiesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20131, 20), 668}, // 748 MaxReplicatesALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20151, 19), 668}, // 749 MaxReplicatesINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21091, 18), 693}, // 750 MediaBlockIOINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19379, 15), 638}, // 751 MemberOffsetNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19935, 13), 662}, // 752 MemoryALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19948, 12), 662}, // 753 MemoryINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20211, 12), 670}, // 754 MergeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20223, 11), 670}, // 755 MergeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20742, 13), 684}, // 756 NoAliasINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18628, 14), 603}, // 757 NoContraction in SPV_OPERAND_TYPE_DECORATION
  {IR(18125, 14), 574}, // 758 NoPerspective in SPV_OPERAND_TYPE_DECORATION
  {IR(18779, 13), 610}, // 759 NoSignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18792, 15), 611}, // 760 NoUnsignedWrap in SPV_OPERAND_TYPE_DECORATION
  {IR(18919, 20), 617}, // 761 NodeMaxPayloadsAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18887, 32), 616}, // 762 NodeSharesPayloadLimitsWithAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18232, 12), 586}, // 763 NonReadable in SPV_OPERAND_TYPE_DECORATION
  {IR(19286, 11), 635}, // 764 NonUniform in SPV_OPERAND_TYPE_DECORATION
  {IR(19297, 14), 635}, // 765 NonUniformEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18220, 12), 585}, // 766 NonWritable in SPV_OPERAND_TYPE_DECORATION
  {IR(19960, 15), 663}, // 767 NumbanksALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19975, 14), 663}, // 768 NumbanksINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18362, 7), 596}, // 769 Offset in SPV_OPERAND_TYPE_DECORATION
  {IR(19104, 12), 625}, // 770 OffsetIdEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19130, 19), 627}, // 771 OverrideCoverageNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19149, 14), 628}, // 772 PassthroughNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18144, 6), 576}, // 773 Patch in SPV_OPERAND_TYPE_DECORATION
  {IR(19059, 28), 623}, // 774 PayloadDispatchIndirectAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19034, 25), 622}, // 775 PayloadNodeArraySizeAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18982, 25), 620}, // 776 PayloadNodeBaseIndexAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(18962, 20), 619}, // 777 PayloadNodeNameAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19007, 27), 621}, // 778 PayloadNodeSparseArrayAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(19210, 16), 631}, // 779 PerPrimitiveEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19226, 15), 631}, // 780 PerPrimitiveNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19251, 10), 633}, // 781 PerTaskNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19261, 13), 634}, // 782 PerVertexKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(19274, 12), 634}, // 783 PerVertexNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19241, 10), 632}, // 784 PerViewNV in SPV_OPERAND_TYPE_DECORATION
  {IR(20845, 21), 687}, // 785 PipelineEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20866, 20), 687}, // 786 PipelineEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20572, 15), 679}, // 787 PrefetchALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20587, 14), 679}, // 788 PrefetchINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19568, 26), 648}, // 789 ReferencedIndirectlyINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19906, 15), 661}, // 790 RegisterALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(19921, 14), 661}, // 791 RegisterINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21364, 32), 699}, // 792 RegisterMapKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21396, 31), 699}, // 793 RegisterMapKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17976, 17), 562}, // 794 RelaxedPrecision in SPV_OPERAND_TYPE_DECORATION
  {IR(18176, 9), 580}, // 795 Restrict in SPV_OPERAND_TYPE_DECORATION
  {IR(19311, 16), 636}, // 796 RestrictPointer in SPV_OPERAND_TYPE_DECORATION
  {IR(19327, 19), 636}, // 797 RestrictPointerEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(18018, 9), 566}, // 798 RowMajor in SPV_OPERAND_TYPE_DECORATION
  {IR(19554, 14), 647}, // 799 SIMTCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18159, 7), 578}, // 800 Sample in SPV_OPERAND_TYPE_DECORATION
  {IR(18288, 20), 589}, // 801 SaturatedConversion in SPV_OPERAND_TYPE_DECORATION
  {IR(18735, 44), 609}, // 802 SaturatedToLargestFloat8NormalConversionEXT in SPV_OPERAND_TYPE_DECORATION
  {IR(19182, 28), 630}, // 803 SecondaryViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(19607, 17), 650}, // 804 SideEffectsINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20170, 21), 669}, // 805 SimpleDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20191, 20), 669}, // 806 SimpleDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21031, 25), 691}, // 807 SingleElementVectorINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20065, 17), 666}, // 808 SinglepumpALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20082, 16), 666}, // 809 SinglepumpINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(17993, 7), 563}, // 810 SpecId in SPV_OPERAND_TYPE_DECORATION
  {IR(21839, 27), 706}, // 811 StableKernelArgumentALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21866, 26), 706}, // 812 StableKernelArgumentINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19699, 15), 654}, // 813 StackCallINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(20601, 18), 680}, // 814 StallEnableALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20619, 17), 680}, // 815 StallEnableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(21109, 16), 694}, // 816 StallFreeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(21125, 15), 694}, // 817 StallFreeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18308, 7), 590}, // 818 Stream in SPV_OPERAND_TYPE_DECORATION
  {IR(20346, 17), 673}, // 819 StridesizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20363, 16), 673}, // 820 StridesizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18939, 23), 618}, // 821 TrackFinishWritingAMDX in SPV_OPERAND_TYPE_DECORATION
  {IR(20408, 19), 675}, // 822 TrueDualPortALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20427, 18), 675}, // 823 TrueDualPortINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19116, 14), 626}, // 824 UTFEncodedKHR in SPV_OPERAND_TYPE_DECORATION
  {IR(18244, 8), 587}, // 825 Uniform in SPV_OPERAND_TYPE_DECORATION
  {IR(18278, 10), 588}, // 826 UniformId in SPV_OPERAND_TYPE_DECORATION
  {IR(19778, 13), 657}, // 827 UserSemantic in SPV_OPERAND_TYPE_DECORATION
  {IR(19810, 15), 658}, // 828 UserTypeGOOGLE in SPV_OPERAND_TYPE_DECORATION
  {IR(21056, 35), 692}, // 829 VectorComputeCallableFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19672, 27), 653}, // 830 VectorComputeFunctionINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19624, 27), 651}, // 831 VectorComputeVariableINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(19163, 19), 629}, // 832 ViewportRelativeNV in SPV_OPERAND_TYPE_DECORATION
  {IR(18193, 9), 582}, // 833 Volatile in SPV_OPERAND_TYPE_DECORATION
  {IR(18807, 18), 612}, // 834 WeightTextureQCOM in SPV_OPERAND_TYPE_DECORATION
  {IR(20379, 15), 674}, // 835 WordsizeALTERA in SPV_OPERAND_TYPE_DECORATION
  {IR(20394, 14), 674}, // 836 WordsizeINTEL in SPV_OPERAND_TYPE_DECORATION
  {IR(18369, 10), 597}, // 837 XfbBuffer in SPV_OPERAND_TYPE_DECORATION
  {IR(18379, 10), 598}, // 838 XfbStride in SPV_OPERAND_TYPE_DECORATION
  {IR(22215, 3), 714}, // 839 1D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22218, 3), 715}, // 840 2D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22221, 3), 716}, // 841 3D in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22234, 7), 719}, // 842 Buffer in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22224, 5), 717}, // 843 Cube in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22229, 5), 718}, // 844 Rect in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22241, 12), 720}, // 845 SubpassData in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(22253, 17), 721}, // 846 TileImageDataEXT in SPV_OPERAND_TYPE_DIMENSIONALITY
  {IR(23413, 20), 791}, // 847 ArithmeticPoisonKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23107, 15), 776}, // 848 CoalescingAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22696, 15), 752}, // 849 ContractionOff in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11068, 18), 766}, // 850 DenormFlushToZero in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11053, 15), 765}, // 851 DenormPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22466, 13), 735}, // 852 DepthGreater in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22479, 10), 736}, // 853 DepthLess in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22451, 15), 734}, // 854 DepthReplacing in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22489, 15), 737}, // 855 DepthUnchanged in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23548, 25), 795}, // 856 DerivativeGroupLinearKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23573, 24), 795}, // 857 DerivativeGroupLinearNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23501, 24), 794}, // 858 DerivativeGroupQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23525, 23), 794}, // 859 DerivativeGroupQuadsNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23055, 29), 774}, // 860 EarlyAndLateFragmentTestsAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22418, 19), 731}, // 861 EarlyFragmentTests in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24058, 18), 815}, // 862 FPFastMathDefault in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22723, 10), 754}, // 863 Finalizer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23873, 26), 807}, // 864 FloatingPointModeALTINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23899, 27), 808}, // 865 FloatingPointModeIEEEINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22711, 12), 753}, // 866 Initializer in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22540, 11), 741}, // 867 InputLines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22551, 20), 742}, // 868 InputLinesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22528, 12), 740}, // 869 InputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22581, 24), 744}, // 870 InputTrianglesAdjacency in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22270, 12), 722}, // 871 Invocations in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23122, 15), 777}, // 872 IsApiEntryAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22611, 9), 746}, // 873 Isolines in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22504, 10), 738}, // 874 LocalSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22514, 14), 739}, // 875 LocalSizeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22791, 16), 759}, // 876 LocalSizeHintId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22779, 12), 758}, // 877 LocalSizeId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23137, 21), 778}, // 878 MaxNodeRecursionAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23182, 21), 781}, // 879 MaxNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23948, 16), 810}, // 880 MaxWorkDimINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23926, 22), 809}, // 881 MaxWorkgroupSizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24034, 24), 814}, // 882 MaximallyReconvergesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24170, 22), 820}, // 883 MaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24192, 24), 821}, // 884 MaximumRegistersIdINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24147, 23), 819}, // 885 NamedBarrierCountINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24267, 27), 822}, // 886 NamedMaximumRegistersINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23964, 20), 811}, // 887 NoGlobalOffsetINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22807, 34), 760}, // 888 NonCoherentColorAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22841, 34), 761}, // 889 NonCoherentDepthAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22875, 36), 762}, // 890 NonCoherentStencilAttachmentReadEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22959, 34), 770}, // 891 NonCoherentTileAttachmentReadQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23984, 22), 812}, // 892 NumSIMDWorkitemsINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24076, 21), 816}, // 893 OpacityMicromapIdKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22402, 16), 730}, // 894 OriginLowerLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22386, 16), 729}, // 895 OriginUpperLeft in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22648, 16), 749}, // 896 OutputLineStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23433, 15), 792}, // 897 OutputLinesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23448, 14), 792}, // 898 OutputLinesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22635, 13), 748}, // 899 OutputPoints in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23462, 20), 793}, // 900 OutputPrimitivesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23482, 19), 793}, // 901 OutputPrimitivesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22664, 20), 750}, // 902 OutputTriangleStrip in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23597, 19), 796}, // 903 OutputTrianglesEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23616, 18), 796}, // 904 OutputTrianglesNV in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22620, 15), 747}, // 905 OutputVertices in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22367, 19), 728}, // 906 PixelCenterInteger in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23634, 25), 797}, // 907 PixelInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23659, 27), 798}, // 908 PixelInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22437, 10), 732}, // 909 PointMode in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22941, 18), 764}, // 910 PostDepthCoverage in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23354, 19), 788}, // 911 QuadDerivativesKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22605, 6), 745}, // 912 Quads in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24121, 26), 818}, // 913 RegisterMapInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23373, 20), 789}, // 914 RequireFullQuadsKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11111, 16), 768}, // 915 RoundingModeRTE in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23852, 21), 806}, // 916 RoundingModeRTNINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23831, 21), 805}, // 917 RoundingModeRTPINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11127, 16), 769}, // 918 RoundingModeRTZ in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23686, 26), 799}, // 919 SampleInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23712, 28), 800}, // 920 SampleInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24006, 28), 813}, // 921 SchedulerTargetFmaxMhzINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(13554, 23), 803}, // 922 Shader64BitIndexingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(7585, 16), 780}, // 923 ShaderIndexAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23740, 31), 801}, // 924 ShadingRateInterlockOrderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23771, 33), 802}, // 925 ShadingRateInterlockUnorderedEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23804, 27), 804}, // 926 SharedLocalMemorySizeINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23393, 20), 790}, // 927 SharesInputWithAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(11086, 25), 767}, // 928 SignedZeroInfNanPreserve in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22282, 13), 723}, // 929 SpacingEqual in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22295, 22), 724}, // 930 SpacingFractionalEven in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22317, 21), 725}, // 931 SpacingFractionalOdd in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23158, 24), 779}, // 932 StaticNumWorkgroupsAMDX in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23307, 25), 786}, // 933 StencilRefGreaterBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23231, 26), 783}, // 934 StencilRefGreaterFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23332, 22), 787}, // 935 StencilRefLessBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23257, 23), 784}, // 936 StencilRefLessFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23084, 23), 775}, // 937 StencilRefReplacingEXT in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23280, 27), 785}, // 938 StencilRefUnchangedBackAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23203, 28), 782}, // 939 StencilRefUnchangedFrontAMD in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24097, 24), 817}, // 940 StreamingInterfaceINTEL in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(6692, 13), 755}, // 941 SubgroupSize in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23034, 21), 773}, // 942 SubgroupSizeFullQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(23013, 21), 772}, // 943 SubgroupSizeHalfQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22911, 30), 763}, // 944 SubgroupUniformControlFlowKHR in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22733, 22), 756}, // 945 SubgroupsPerWorkgroup in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22755, 24), 757}, // 946 SubgroupsPerWorkgroupId in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22993, 20), 771}, // 947 TileShadingRateQCOM in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22571, 10), 743}, // 948 Triangles in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22684, 12), 751}, // 949 VecTypeHint in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22352, 15), 727}, // 950 VertexOrderCcw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22338, 14), 726}, // 951 VertexOrderCw in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(22447, 4), 733}, // 952 Xfb in SPV_OPERAND_TYPE_EXECUTION_MODE
  {IR(24432, 10), 834}, // 953 AnyHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24442, 9), 834}, // 954 AnyHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24493, 12), 837}, // 955 CallableKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24505, 11), 837}, // 956 CallableNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24451, 14), 835}, // 957 ClosestHitKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24465, 13), 835}, // 958 ClosestHitNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(17359, 9), 827}, // 959 Fragment in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24344, 10), 828}, // 960 GLCompute in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(6173, 9), 826}, // 961 Geometry in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24401, 16), 833}, // 962 IntersectionKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24417, 15), 833}, // 963 IntersectionNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(5940, 7), 829}, // 964 Kernel in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24524, 8), 839}, // 965 MeshEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24361, 7), 831}, // 966 MeshNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24478, 8), 836}, // 967 MissKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24486, 7), 836}, // 968 MissNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24368, 17), 832}, // 969 RayGenerationKHR in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24385, 16), 832}, // 970 RayGenerationNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24516, 8), 838}, // 971 TaskEXT in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24354, 7), 830}, // 972 TaskNV in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24301, 20), 824}, // 973 TessellationControl in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24321, 23), 825}, // 974 TessellationEvaluation in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24294, 7), 823}, // 975 Vertex in SPV_OPERAND_TYPE_EXECUTION_MODEL
  {IR(24541, 12), 841}, // 976 FlushToZero in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(24532, 9), 840}, // 977 Preserve in SPV_OPERAND_TYPE_FPDENORM_MODE
  {IR(24553, 12), 842}, // 978 BFloat16KHR in SPV_OPERAND_TYPE_FPENCODING
  {IR(24621, 14), 847}, // 979 Float4E2M1EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24593, 14), 845}, // 980 Float6E2M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24607, 14), 846}, // 981 Float6E3M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24565, 14), 843}, // 982 Float8E4M3EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24579, 14), 844}, // 983 Float8E5M2EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(10548, 22), 848}, // 984 Float8UnsignedE8M0EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(10570, 10), 849}, // 985 MXInt8EXT in SPV_OPERAND_TYPE_FPENCODING
  {IR(24640, 4), 851}, // 986 ALT in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(24635, 5), 850}, // 987 IEEE in SPV_OPERAND_TYPE_FPOPERATION_MODE
  {IR(24678, 14), 858}, // 988 AllowContract in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24692, 23), 858}, // 989 AllowContractFastINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24715, 13), 859}, // 990 AllowReassoc in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24728, 18), 859}, // 991 AllowReassocINTEL in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24662, 11), 856}, // 992 AllowRecip in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24746, 15), 860}, // 993 AllowTransform in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24673, 5), 857}, // 994 Fast in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24658, 4), 855}, // 995 NSZ in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(16955, 5), 852}, // 996 None in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24651, 7), 854}, // 997 NotInf in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24644, 7), 853}, // 998 NotNaN in SPV_OPERAND_TYPE_FP_FAST_MATH_MODE
  {IR(24761, 4), 861}, // 999 RTE in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24773, 4), 864}, // 1000 RTN in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24769, 4), 863}, // 1001 RTP in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24765, 4), 862}, // 1002 RTZ in SPV_OPERAND_TYPE_FP_ROUNDING_MODE
  {IR(24809, 18), 867}, // 1003 Horizontal2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24827, 18), 868}, // 1004 Horizontal4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24777, 16), 865}, // 1005 Vertical2Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24793, 16), 866}, // 1006 Vertical4Pixels in SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE
  {IR(24868, 6), 873}, // 1007 Const in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24852, 11), 871}, // 1008 DontInline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24845, 7), 870}, // 1009 Inline in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(16955, 5), 869}, // 1010 None in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15763, 11), 874}, // 1011 OptNoneEXT in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(15774, 13), 874}, // 1012 OptNoneINTEL in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24863, 5), 872}, // 1013 Pure in SPV_OPERAND_TYPE_FUNCTION_CONTROL
  {IR(24884, 6), 877}, // 1014 ByVal in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24895, 8), 879}, // 1015 NoAlias in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24903, 10), 880}, // 1016 NoCapture in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24921, 12), 882}, // 1017 NoReadWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24913, 8), 881}, // 1018 NoWrite in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24933, 21), 883}, // 1019 RuntimeAlignedALTERA in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24954, 20), 883}, // 1020 RuntimeAlignedINTEL in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24879, 5), 876}, // 1021 Sext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24890, 5), 878}, // 1022 Sret in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24874, 5), 875}, // 1023 Zext in SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE
  {IR(24974, 14), 884}, // 1024 Gather4x1QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(24988, 12), 885}, // 1025 GatherDQCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(25000, 13), 886}, // 1026 GatherH2QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(25013, 13), 887}, // 1027 GatherV2QCOM in SPV_OPERAND_TYPE_GATHER_MODES
  {IR(25061, 16), 891}, // 1028 ClusteredReduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25047, 14), 890}, // 1029 ExclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25033, 14), 889}, // 1030 InclusiveScan in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25173, 28), 894}, // 1031 PartitionedExclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25201, 27), 894}, // 1032 PartitionedExclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25118, 28), 893}, // 1033 PartitionedInclusiveScanEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25146, 27), 893}, // 1034 PartitionedInclusiveScanNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25077, 21), 892}, // 1035 PartitionedReduceEXT in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25098, 20), 892}, // 1036 PartitionedReduceNV in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25026, 7), 888}, // 1037 Reduce in SPV_OPERAND_TYPE_GROUP_OPERATION
  {IR(25228, 10), 895}, // 1038 NoneINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25238, 10), 896}, // 1039 ReadINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25259, 15), 898}, // 1040 ReadWriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25248, 11), 897}, // 1041 WriteINTEL in SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER
  {IR(25274, 5), 900}, // 1042 Bias in SPV_OPERAND_TYPE_IMAGE
  {IR(25288, 12), 903}, // 1043 ConstOffset in SPV_OPERAND_TYPE_IMAGE
  {IR(25300, 13), 905}, // 1044 ConstOffsets in SPV_OPERAND_TYPE_IMAGE
  {IR(25283, 5), 902}, // 1045 Grad in SPV_OPERAND_TYPE_IMAGE
  {IR(25279, 4), 901}, // 1046 Lod in SPV_OPERAND_TYPE_IMAGE
  {IR(25313, 19), 908}, // 1047 MakeTexelAvailable in SPV_OPERAND_TYPE_IMAGE
  {IR(25332, 22), 908}, // 1048 MakeTexelAvailableKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(25354, 17), 909}, // 1049 MakeTexelVisible in SPV_OPERAND_TYPE_IMAGE
  {IR(25371, 20), 909}, // 1050 MakeTexelVisibleKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(9816, 7), 907}, // 1051 MinLod in SPV_OPERAND_TYPE_IMAGE
  {IR(25391, 16), 910}, // 1052 NonPrivateTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(25407, 19), 910}, // 1053 NonPrivateTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(16955, 5), 899}, // 1054 None in SPV_OPERAND_TYPE_IMAGE
  {IR(25479, 12), 914}, // 1055 Nontemporal in SPV_OPERAND_TYPE_IMAGE
  {IR(18362, 7), 904}, // 1056 Offset in SPV_OPERAND_TYPE_IMAGE
  {IR(25491, 8), 915}, // 1057 Offsets in SPV_OPERAND_TYPE_IMAGE
  {IR(18159, 7), 906}, // 1058 Sample in SPV_OPERAND_TYPE_IMAGE
  {IR(25457, 11), 912}, // 1059 SignExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(25426, 14), 911}, // 1060 VolatileTexel in SPV_OPERAND_TYPE_IMAGE
  {IR(25440, 17), 911}, // 1061 VolatileTexelKHR in SPV_OPERAND_TYPE_IMAGE
  {IR(25468, 11), 913}, // 1062 ZeroExtend in SPV_OPERAND_TYPE_IMAGE
  {IR(16852, 6), 930}, // 1063 Float in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25660, 10), 929}, // 1064 HalfFloat in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25595, 12), 924}, // 1065 SignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25607, 12), 925}, // 1066 SignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25584, 11), 923}, // 1067 SignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25509, 11), 917}, // 1068 SnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25499, 10), 916}, // 1069 SnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25569, 15), 922}, // 1070 UnormInt101010 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25681, 17), 932}, // 1071 UnormInt101010_2 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25698, 16), 933}, // 1072 UnormInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25831, 16), 940}, // 1073 UnormInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25847, 16), 941}, // 1074 UnormInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25530, 11), 919}, // 1075 UnormInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25670, 11), 931}, // 1076 UnormInt24 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25754, 20), 936}, // 1077 UnormInt2_101010EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25520, 10), 918}, // 1078 UnormInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25555, 14), 921}, // 1079 UnormShort555 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25541, 14), 920}, // 1080 UnormShort565 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25774, 19), 937}, // 1081 UnsignedInt10X6EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25793, 19), 938}, // 1082 UnsignedInt12X4EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25812, 19), 939}, // 1083 UnsignedInt14X2EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25632, 14), 927}, // 1084 UnsignedInt16 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25646, 14), 928}, // 1085 UnsignedInt32 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25619, 13), 926}, // 1086 UnsignedInt8 in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25714, 20), 934}, // 1087 UnsignedIntRaw10EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25734, 20), 935}, // 1088 UnsignedIntRaw12EXT in SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE
  {IR(25865, 2), 943}, // 1089 A in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25966, 5), 961}, // 1090 ABGR in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25887, 5), 949}, // 1091 ARGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25882, 5), 948}, // 1092 BGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25924, 6), 955}, // 1093 Depth in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25930, 13), 956}, // 1094 DepthStencil in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25892, 10), 950}, // 1095 Intensity in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25902, 10), 951}, // 1096 Luminance in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25863, 2), 942}, // 1097 R in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25870, 3), 945}, // 1098 RA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25867, 3), 944}, // 1099 RG in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25873, 4), 946}, // 1100 RGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25877, 5), 947}, // 1101 RGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25919, 5), 954}, // 1102 RGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25915, 4), 953}, // 1103 RGx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25912, 3), 952}, // 1104 Rx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25960, 6), 960}, // 1105 sBGRA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25943, 5), 957}, // 1106 sRGB in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25954, 6), 959}, // 1107 sRGBA in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25948, 6), 958}, // 1108 sRGBx in SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER
  {IR(25971, 28), 962}, // 1109 InitOnDeviceReprogramALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(25999, 27), 962}, // 1110 InitOnDeviceReprogramINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(26026, 24), 963}, // 1111 InitOnDeviceResetALTERA in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(26050, 23), 963}, // 1112 InitOnDeviceResetINTEL in SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER
  {IR(26073, 7), 964}, // 1113 NoWait in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(26080, 11), 965}, // 1114 WaitKernel in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(26091, 14), 966}, // 1115 WaitWorkGroup in SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS
  {IR(26105, 12), 968}, // 1116 CmdExecTime in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(16955, 5), 967}, // 1117 None in SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO
  {IR(26117, 13), 970}, // 1118 MayUsePrintf in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(16955, 5), 969}, // 1119 None in SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS
  {IR(26130, 7), 971}, // 1120 Export in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(26137, 7), 972}, // 1121 Import in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(26144, 12), 973}, // 1122 LinkOnceODR in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(26156, 8), 974}, // 1123 WeakAMD in SPV_OPERAND_TYPE_LINKAGE_TYPE
  {IR(26178, 12), 976}, // 1124 CachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26230, 17), 979}, // 1125 ConstCachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26205, 25), 978}, // 1126 InvalidateAfterReadINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26190, 15), 977}, // 1127 StreamingINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26164, 14), 975}, // 1128 UncachedINTEL in SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL
  {IR(26370, 22), 992}, // 1129 DependencyArrayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26392, 21), 992}, // 1130 DependencyArrayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26265, 19), 983}, // 1131 DependencyInfinite in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26284, 17), 984}, // 1132 DependencyLength in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26254, 11), 982}, // 1133 DontUnroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20755, 25), 990}, // 1134 InitiationIntervalALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20780, 24), 990}, // 1135 InitiationIntervalINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26329, 18), 987}, // 1136 IterationMultiple in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26413, 19), 994}, // 1137 LoopCoalesceALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26432, 18), 994}, // 1138 LoopCoalesceINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26575, 16), 998}, // 1139 LoopCountALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26591, 15), 998}, // 1140 LoopCountINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20804, 21), 991}, // 1141 MaxConcurrencyALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20825, 20), 991}, // 1142 MaxConcurrencyINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26450, 22), 995}, // 1143 MaxInterleavingALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26472, 21), 995}, // 1144 MaxInterleavingINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26315, 14), 986}, // 1145 MaxIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26606, 27), 999}, // 1146 MaxReinvocationDelayALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26633, 26), 999}, // 1147 MaxReinvocationDelayINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26301, 14), 985}, // 1148 MinIterations in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(11389, 23), 1000}, // 1149 MultipleWaitQueuesQCOM in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26546, 15), 997}, // 1150 NoFusionALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26561, 14), 997}, // 1151 NoFusionINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(16955, 5), 980}, // 1152 None in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26357, 13), 989}, // 1153 PartialCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26347, 10), 988}, // 1154 PeelCount in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20845, 21), 993}, // 1155 PipelineEnableALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(20866, 20), 993}, // 1156 PipelineEnableINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26493, 27), 996}, // 1157 SpeculatedIterationsALTERA in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26520, 26), 996}, // 1158 SpeculatedIterationsINTEL in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26247, 7), 981}, // 1159 Unroll in SPV_OPERAND_TYPE_LOOP_CONTROL
  {IR(26942, 27), 1014}, // 1160 MatrixAPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26890, 26), 1012}, // 1161 MatrixAPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26810, 23), 1008}, // 1162 MatrixAPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26764, 23), 1006}, // 1163 MatrixAPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26659, 29), 1002}, // 1164 MatrixASignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26856, 17), 1010}, // 1165 MatrixATF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26969, 27), 1015}, // 1166 MatrixBPackedBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26916, 26), 1013}, // 1167 MatrixBPackedFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26833, 23), 1009}, // 1168 MatrixBPackedInt4INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26787, 23), 1007}, // 1169 MatrixBPackedInt8INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26688, 29), 1003}, // 1170 MatrixBSignedComponentsINTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26873, 17), 1011}, // 1171 MatrixBTF32INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26717, 21), 1004}, // 1172 MatrixCBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(26738, 26), 1005}, // 1173 MatrixResultBFloat16INTEL in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(16955, 5), 1001}, // 1174 None in SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS
  {IR(27129, 20), 1023}, // 1175 AliasScopeINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(26996, 8), 1018}, // 1176 Aligned in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27004, 21), 1020}, // 1177 MakePointerAvailable in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27025, 24), 1020}, // 1178 MakePointerAvailableKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27049, 19), 1021}, // 1179 MakePointerVisible in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27068, 22), 1021}, // 1180 MakePointerVisibleKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27149, 17), 1024}, // 1181 NoAliasINTELMask in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27090, 18), 1022}, // 1182 NonPrivatePointer in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27108, 21), 1022}, // 1183 NonPrivatePointerKHR in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(16955, 5), 1016}, // 1184 None in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(25479, 12), 1019}, // 1185 Nontemporal in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(18193, 9), 1017}, // 1186 Volatile in SPV_OPERAND_TYPE_MEMORY_ACCESS
  {IR(27173, 8), 1026}, // 1187 GLSL450 in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27181, 7), 1027}, // 1188 OpenCL in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27166, 7), 1025}, // 1189 Simple in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27188, 7), 1028}, // 1190 Vulkan in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27195, 10), 1028}, // 1191 VulkanKHR in SPV_OPERAND_TYPE_MEMORY_MODEL
  {IR(27213, 8), 1030}, // 1192 Acquire in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27229, 15), 1032}, // 1193 AcquireRelease in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27333, 20), 1038}, // 1194 AtomicCounterMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27312, 21), 1037}, // 1195 CrossWorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27353, 12), 1039}, // 1196 ImageMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27394, 14), 1041}, // 1197 MakeAvailable in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27408, 17), 1041}, // 1198 MakeAvailableKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27425, 12), 1042}, // 1199 MakeVisible in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27437, 15), 1042}, // 1200 MakeVisibleKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(16955, 5), 1029}, // 1201 None in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27365, 13), 1040}, // 1202 OutputMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27378, 16), 1040}, // 1203 OutputMemoryKHR in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27205, 8), 1029}, // 1204 Relaxed in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27221, 8), 1031}, // 1205 Release in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27244, 23), 1033}, // 1206 SequentiallyConsistent in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27281, 15), 1035}, // 1207 SubgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27267, 14), 1034}, // 1208 UniformMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(18193, 9), 1043}, // 1209 Volatile in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27296, 16), 1036}, // 1210 WorkgroupMemory in SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID
  {IR(27452, 10), 1044}, // 1211 AutoINTEL in SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS
  {IR(27467, 4), 1046}, // 1212 SAT in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27480, 8), 1048}, // 1213 SAT_SYM in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27471, 9), 1047}, // 1214 SAT_ZERO in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27462, 5), 1045}, // 1215 WRAP in SPV_OPERAND_TYPE_OVERFLOW_MODES
  {IR(27488, 25), 1049}, // 1216 PackedVectorFormat4x8Bit in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(27513, 28), 1049}, // 1217 PackedVectorFormat4x8BitKHR in SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT
  {IR(27554, 4), 1052}, // 1218 RND in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27587, 9), 1056}, // 1219 RND_CONV in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27596, 13), 1057}, // 1220 RND_CONV_ODD in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27567, 8), 1054}, // 1221 RND_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27575, 12), 1055}, // 1222 RND_MIN_INF in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27558, 9), 1053}, // 1223 RND_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27541, 4), 1050}, // 1224 TRN in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(27545, 9), 1051}, // 1225 TRN_ZERO in SPV_OPERAND_TYPE_QUANTIZATION_MODES
  {IR(16955, 5), 1058}, // 1226 None in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27609, 25), 1059}, // 1227 RobustnessPerComponentNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27634, 23), 1060}, // 1228 RobustnessPerElementNV in SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS
  {IR(27726, 27), 1066}, // 1229 CullBackFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27753, 28), 1067}, // 1230 CullFrontFacingTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27795, 16), 1069}, // 1231 CullNoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27781, 14), 1068}, // 1232 CullOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27895, 30), 1072}, // 1233 ForceOpacityMicromap2StateEXT in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27865, 30), 1072}, // 1234 ForceOpacityMicromap2StateKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27667, 12), 1063}, // 1235 NoOpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(17707, 8), 1061}, // 1236 NoneKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27657, 10), 1062}, // 1237 OpaqueKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27852, 13), 1071}, // 1238 SkipAABBsKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27828, 24), 1070}, // 1239 SkipBuiltinPrimitivesNV in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27702, 24), 1065}, // 1240 SkipClosestHitShaderKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27811, 17), 1070}, // 1241 SkipTrianglesKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27679, 23), 1064}, // 1242 TerminateOnFirstHitKHR in SPV_OPERAND_TYPE_RAY_FLAGS
  {IR(27966, 37), 1074}, // 1243 RayQueryCandidateIntersectionAABBKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(27925, 41), 1073}, // 1244 RayQueryCandidateIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE
  {IR(28081, 42), 1077}, // 1245 RayQueryCommittedIntersectionGeneratedKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(28003, 37), 1075}, // 1246 RayQueryCommittedIntersectionNoneKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(28040, 41), 1076}, // 1247 RayQueryCommittedIntersectionTriangleKHR in SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE
  {IR(28123, 33), 1078}, // 1248 RayQueryCandidateIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(28156, 33), 1079}, // 1249 RayQueryCommittedIntersectionKHR in SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION
  {IR(28201, 6), 1082}, // 1250 Clamp in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28189, 12), 1081}, // 1251 ClampToEdge in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(16955, 5), 1080}, // 1252 None in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28207, 7), 1083}, // 1253 Repeat in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28214, 15), 1084}, // 1254 RepeatMirrored in SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE
  {IR(28237, 7), 1086}, // 1255 Linear in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(28229, 8), 1085}, // 1256 Nearest in SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE
  {IR(28302, 13), 1095}, // 1257 R11fG11fB10f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28344, 4), 1101}, // 1258 R16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28382, 9), 1106}, // 1259 R16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28315, 5), 1096}, // 1260 R16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28444, 5), 1115}, // 1261 R16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28515, 6), 1125}, // 1262 R16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28268, 5), 1090}, // 1263 R32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28422, 5), 1111}, // 1264 R32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28479, 6), 1120}, // 1265 R32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28532, 5), 1128}, // 1266 R64i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28526, 6), 1127}, // 1267 R64ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28348, 3), 1102}, // 1268 R8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28391, 8), 1107}, // 1269 R8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28449, 4), 1116}, // 1270 R8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28521, 5), 1126}, // 1271 R8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28335, 5), 1099}, // 1272 Rg16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28363, 10), 1104}, // 1273 Rg16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28296, 6), 1094}, // 1274 Rg16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28433, 6), 1113}, // 1275 Rg16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28502, 7), 1123}, // 1276 Rg16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28290, 6), 1093}, // 1277 Rg32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28427, 6), 1112}, // 1278 Rg32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28495, 7), 1122}, // 1279 Rg32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28340, 4), 1100}, // 1280 Rg8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28373, 9), 1105}, // 1281 Rg8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28439, 5), 1114}, // 1282 Rg8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28509, 6), 1124}, // 1283 Rg8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28327, 8), 1098}, // 1284 Rgb10A2 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28485, 10), 1121}, // 1285 Rgb10a2ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28320, 7), 1097}, // 1286 Rgba16 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28351, 12), 1103}, // 1287 Rgba16Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28260, 8), 1089}, // 1288 Rgba16f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28407, 8), 1109}, // 1289 Rgba16i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28462, 9), 1118}, // 1290 Rgba16ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28252, 8), 1088}, // 1291 Rgba32f in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28399, 8), 1108}, // 1292 Rgba32i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28453, 9), 1117}, // 1293 Rgba32ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28273, 6), 1091}, // 1294 Rgba8 in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28279, 11), 1092}, // 1295 Rgba8Snorm in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28415, 7), 1110}, // 1296 Rgba8i in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28471, 8), 1119}, // 1297 Rgba8ui in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28244, 8), 1087}, // 1298 Unknown in SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT
  {IR(28537, 12), 1129}, // 1299 CrossDevice in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28549, 7), 1130}, // 1300 Device in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28575, 11), 1133}, // 1301 Invocation in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28586, 12), 1134}, // 1302 QueueFamily in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28598, 15), 1134}, // 1303 QueueFamilyKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28613, 14), 1135}, // 1304 ShaderCallKHR in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28566, 9), 1132}, // 1305 Subgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28556, 10), 1131}, // 1306 Workgroup in SPV_OPERAND_TYPE_SCOPE_ID
  {IR(28635, 12), 1138}, // 1307 DontFlatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(28627, 8), 1137}, // 1308 Flatten in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(16955, 5), 1136}, // 1309 None in SPV_OPERAND_TYPE_SELECTION_CONTROL
  {IR(28647, 29), 1139}, // 1310 IdentifierPossibleDuplicates in SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS
  {IR(16836, 8), 1141}, // 1311 Address in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16844, 8), 1142}, // 1312 Boolean in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16852, 6), 1143}, // 1313 Float in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16858, 7), 1144}, // 1314 Signed in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16865, 11), 1145}, // 1315 SignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16876, 9), 1146}, // 1316 Unsigned in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16885, 13), 1147}, // 1317 UnsignedChar in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16824, 12), 1140}, // 1318 Unspecified in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING
  {IR(16898, 6), 1148}, // 1319 Class in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16904, 10), 1149}, // 1320 Structure in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16914, 6), 1150}, // 1321 Union in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE
  {IR(16935, 20), 1152}, // 1322 ImportedDeclaration in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(16920, 15), 1151}, // 1323 ImportedModule in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY
  {IR(17044, 15), 1160}, // 1324 FlagArtificial in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17059, 13), 1161}, // 1325 FlagExplicit in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17032, 12), 1159}, // 1326 FlagFwdDecl in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17122, 21), 1165}, // 1327 FlagIndirectVariable in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17015, 17), 1158}, // 1328 FlagIsDefinition in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17199, 16), 1169}, // 1329 FlagIsEnumClass in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17003, 12), 1157}, // 1330 FlagIsLocal in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17183, 16), 1168}, // 1331 FlagIsOptimized in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16976, 14), 1155}, // 1332 FlagIsPrivate in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16960, 16), 1154}, // 1333 FlagIsProtected in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16990, 13), 1156}, // 1334 FlagIsPublic in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17143, 20), 1166}, // 1335 FlagLValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17087, 18), 1163}, // 1336 FlagObjectPointer in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17072, 15), 1162}, // 1337 FlagPrototyped in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17163, 20), 1167}, // 1338 FlagRValueReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17105, 17), 1164}, // 1339 FlagStaticMember in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17235, 24), 1171}, // 1340 FlagTypePassByReference in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17215, 20), 1170}, // 1341 FlagTypePassByValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(28676, 26), 1172}, // 1342 FlagUnknownPhysicalLayout in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(16955, 5), 1153}, // 1343 None in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS
  {IR(17320, 9), 1177}, // 1344 BitPiece in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17352, 7), 1181}, // 1345 Constu in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17259, 6), 1173}, // 1346 Deref in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17359, 9), 1182}, // 1347 Fragment in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17270, 6), 1175}, // 1348 Minus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17265, 5), 1174}, // 1349 Plus in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17309, 11), 1176}, // 1350 PlusUconst in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17341, 11), 1180}, // 1351 StackValue in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17329, 5), 1178}, // 1352 Swap in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17334, 7), 1179}, // 1353 Xderef in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION
  {IR(17404, 11), 1186}, // 1354 AtomicType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17368, 10), 1183}, // 1355 ConstType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17391, 13), 1185}, // 1356 RestrictType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(17378, 13), 1184}, // 1357 VolatileType in SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER
  {IR(28794, 9), 1202}, // 1358 ApilaJai in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28737, 15), 1193}, // 1359 CPP_for_OpenCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28702, 5), 1188}, // 1360 ESSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28707, 5), 1189}, // 1361 GLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28757, 7), 1195}, // 1362 HERO_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28732, 5), 1192}, // 1363 HLSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28764, 5), 1196}, // 1364 NZSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28712, 9), 1190}, // 1365 OpenCL_C in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28721, 11), 1191}, // 1366 OpenCL_CPP in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28789, 5), 1201}, // 1367 Pred in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28784, 5), 1200}, // 1368 Rust in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28752, 5), 1194}, // 1369 SYCL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28774, 6), 1198}, // 1370 Slang in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28244, 8), 1187}, // 1371 Unknown in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28769, 5), 1197}, // 1372 WGSL in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28780, 4), 1199}, // 1373 Zig in SPV_OPERAND_TYPE_SOURCE_LANGUAGE
  {IR(28885, 14), 1213}, // 1374 AtomicCounter in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28967, 16), 1219}, // 1375 CallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28983, 15), 1219}, // 1376 CallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29303, 17), 1229}, // 1377 CodeSectionINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28832, 15), 1208}, // 1378 CrossWorkgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29320, 17), 1230}, // 1379 DeviceOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29337, 16), 1230}, // 1380 DeviceOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28855, 9), 1210}, // 1381 Function in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28864, 8), 1211}, // 1382 Generic in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29072, 16), 1222}, // 1383 HitAttributeKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29088, 15), 1222}, // 1384 HitAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29281, 22), 1228}, // 1385 HitObjectAttributeEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29236, 21), 1226}, // 1386 HitObjectAttributeNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29353, 15), 1231}, // 1387 HostOnlyALTERA in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29368, 14), 1231}, // 1388 HostOnlyINTEL in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28899, 6), 1214}, // 1389 Image in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28998, 24), 1220}, // 1390 IncomingCallableDataKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29022, 23), 1220}, // 1391 IncomingCallableDataNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29103, 22), 1223}, // 1392 IncomingRayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29125, 21), 1223}, // 1393 IncomingRayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28819, 6), 1204}, // 1394 Input in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28951, 16), 1218}, // 1395 NodePayloadAMDX in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28825, 7), 1206}, // 1396 Output in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29189, 22), 1225}, // 1397 PhysicalStorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29211, 25), 1225}, // 1398 PhysicalStorageBufferEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28847, 8), 1209}, // 1399 Private in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28872, 13), 1212}, // 1400 PushConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29045, 14), 1221}, // 1401 RayPayloadKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29059, 13), 1221}, // 1402 RayPayloadNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29146, 22), 1224}, // 1403 ShaderRecordBufferKHR in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29168, 21), 1224}, // 1404 ShaderRecordBufferNV in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28905, 14), 1215}, // 1405 StorageBuffer in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(29257, 24), 1227}, // 1406 TaskPayloadWorkgroupEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28932, 19), 1217}, // 1407 TileAttachmentQCOM in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28919, 13), 1216}, // 1408 TileImageEXT in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(18244, 8), 1205}, // 1409 Uniform in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28803, 16), 1203}, // 1410 UniformConstant in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(28556, 10), 1207}, // 1411 Workgroup in SPV_OPERAND_TYPE_STORAGE_CLASS
  {IR(26190, 15), 1235}, // 1412 StreamingINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(26164, 14), 1232}, // 1413 UncachedINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29400, 15), 1234}, // 1414 WriteBackINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29382, 18), 1233}, // 1415 WriteThroughINTEL in SPV_OPERAND_TYPE_STORE_CACHE_CONTROL
  {IR(29426, 11), 1238}, // 1416 DecodeFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(29437, 17), 1239}, // 1417 DecodeVectorFunc in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(16955, 5), 1236}, // 1418 None in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(29415, 11), 1237}, // 1419 TensorView in SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS
  {IR(28189, 12), 1242}, // 1420 ClampToEdge in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(18202, 9), 1241}, // 1421 Constant in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28207, 7), 1243}, // 1422 Repeat in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(28214, 15), 1244}, // 1423 RepeatMirrored in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(29454, 10), 1240}, // 1424 Undefined in SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE
  {IR(29507, 24), 1248}, // 1425 MakeElementAvailableARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29531, 22), 1249}, // 1426 MakeElementVisibleARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29553, 21), 1250}, // 1427 NonPrivateElementARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29464, 8), 1245}, // 1428 NoneARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29472, 15), 1246}, // 1429 NontemporalARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
  {IR(29487, 20), 1247}, // 1430 OutOfBoundsValueARM in SPV_OPERAND_TYPE_TENSOR_OPERANDS
}};

IndexRange OperandNameRangeForKind(spv_operand_type_t type) {
  switch(type) {
    case SPV_OPERAND_TYPE_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_ADDRESSING_MODEL: return IR(3, 5);
    case SPV_OPERAND_TYPE_BUILT_IN: return IR(8, 151);
    case SPV_OPERAND_TYPE_CAPABILITY: return IR(159, 360);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(519, 8);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(527, 3);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(530, 2);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS: return IR(532, 19);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION: return IR(551, 10);
    case SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(561, 4);
    case SPV_OPERAND_TYPE_COMPONENT_TYPE: return IR(565, 15);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_LAYOUT: return IR(580, 4);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_OPERANDS: return IR(584, 6);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE: return IR(590, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_USE: return IR(593, 3);
    case SPV_OPERAND_TYPE_COOPERATIVE_VECTOR_MATRIX_LAYOUT: return IR(596, 4);
    case SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(600, 8);
    case SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE: return IR(608, 3);
    case SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS: return IR(611, 16);
    case SPV_OPERAND_TYPE_DEBUG_OPERATION: return IR(627, 9);
    case SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER: return IR(636, 3);
    case SPV_OPERAND_TYPE_DECORATION: return IR(639, 200);
    case SPV_OPERAND_TYPE_DIMENSIONALITY: return IR(839, 8);
    case SPV_OPERAND_TYPE_EXECUTION_MODE: return IR(847, 106);
    case SPV_OPERAND_TYPE_EXECUTION_MODEL: return IR(953, 23);
    case SPV_OPERAND_TYPE_FPDENORM_MODE: return IR(976, 2);
    case SPV_OPERAND_TYPE_FPENCODING: return IR(978, 8);
    case SPV_OPERAND_TYPE_FPOPERATION_MODE: return IR(986, 2);
    case SPV_OPERAND_TYPE_FP_FAST_MATH_MODE: return IR(988, 11);
    case SPV_OPERAND_TYPE_FP_ROUNDING_MODE: return IR(999, 4);
    case SPV_OPERAND_TYPE_FRAGMENT_SHADING_RATE: return IR(1003, 4);
    case SPV_OPERAND_TYPE_FUNCTION_CONTROL: return IR(1007, 7);
    case SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE: return IR(1014, 10);
    case SPV_OPERAND_TYPE_GATHER_MODES: return IR(1024, 4);
    case SPV_OPERAND_TYPE_GROUP_OPERATION: return IR(1028, 10);
    case SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER: return IR(1038, 4);
    case SPV_OPERAND_TYPE_IMAGE: return IR(1042, 21);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_DATA_TYPE: return IR(1063, 26);
    case SPV_OPERAND_TYPE_IMAGE_CHANNEL_ORDER: return IR(1089, 20);
    case SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER: return IR(1109, 4);
    case SPV_OPERAND_TYPE_KERNEL_ENQ_FLAGS: return IR(1113, 3);
    case SPV_OPERAND_TYPE_KERNEL_PROFILING_INFO: return IR(1116, 2);
    case SPV_OPERAND_TYPE_KERNEL_PROPERTY_FLAGS: return IR(1118, 2);
    case SPV_OPERAND_TYPE_LINKAGE_TYPE: return IR(1120, 4);
    case SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL: return IR(1124, 5);
    case SPV_OPERAND_TYPE_LOOP_CONTROL: return IR(1129, 31);
    case SPV_OPERAND_TYPE_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1160, 15);
    case SPV_OPERAND_TYPE_MEMORY_ACCESS: return IR(1175, 12);
    case SPV_OPERAND_TYPE_MEMORY_MODEL: return IR(1187, 5);
    case SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID: return IR(1192, 19);
    case SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS: return IR(1211, 1);
    case SPV_OPERAND_TYPE_OVERFLOW_MODES: return IR(1212, 4);
    case SPV_OPERAND_TYPE_PACKED_VECTOR_FORMAT: return IR(1216, 2);
    case SPV_OPERAND_TYPE_QUANTIZATION_MODES: return IR(1218, 8);
    case SPV_OPERAND_TYPE_RAW_ACCESS_CHAIN_OPERANDS: return IR(1226, 3);
    case SPV_OPERAND_TYPE_RAY_FLAGS: return IR(1229, 14);
    case SPV_OPERAND_TYPE_RAY_QUERY_CANDIDATE_INTERSECTION_TYPE: return IR(1243, 2);
    case SPV_OPERAND_TYPE_RAY_QUERY_COMMITTED_INTERSECTION_TYPE: return IR(1245, 3);
    case SPV_OPERAND_TYPE_RAY_QUERY_INTERSECTION: return IR(1248, 2);
    case SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE: return IR(1250, 5);
    case SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE: return IR(1255, 2);
    case SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT: return IR(1257, 42);
    case SPV_OPERAND_TYPE_SCOPE_ID: return IR(1299, 8);
    case SPV_OPERAND_TYPE_SELECTION_CONTROL: return IR(1307, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_BUILD_IDENTIFIER_FLAGS: return IR(1310, 1);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING: return IR(1311, 8);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_COMPOSITE_TYPE: return IR(1319, 3);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_IMPORTED_ENTITY: return IR(1322, 2);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_INFO_FLAGS: return IR(1324, 20);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_OPERATION: return IR(1344, 10);
    case SPV_OPERAND_TYPE_SHDEBUG100_DEBUG_TYPE_QUALIFIER: return IR(1354, 4);
    case SPV_OPERAND_TYPE_SOURCE_LANGUAGE: return IR(1358, 16);
    case SPV_OPERAND_TYPE_STORAGE_CLASS: return IR(1374, 38);
    case SPV_OPERAND_TYPE_STORE_CACHE_CONTROL: return IR(1412, 4);
    case SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS: return IR(1416, 4);
    case SPV_OPERAND_TYPE_TENSOR_CLAMP_MODE: return IR(1420, 5);
    case SPV_OPERAND_TYPE_TENSOR_OPERANDS: return IR(1425, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_IMAGE: return IR(1042, 21);
    case SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER: return IR(0, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS: return IR(1175, 12);
    case SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT: return IR(1216, 2);
    case SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS: return IR(584, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS: return IR(1160, 15);
    case SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS: return IR(1226, 3);
    case SPV_OPERAND_TYPE_OPTIONAL_FPENCODING: return IR(978, 8);
    case SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS: return IR(1425, 6);
    case SPV_OPERAND_TYPE_OPTIONAL_CAPABILITY: return IR(159, 360);
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
static const std::array<InstructionDesc, 877> kInstructionDesc{{
  {spv::Op::OpNop, false, false, IR(0, 0), IR(29574, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpUndef, true, true, IR(39, 2), IR(29630, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpSourceContinued, false, false, IR(11, 1), IR(29636, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSource, false, false, IR(41, 4), IR(29755, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpSourceExtension, false, false, IR(11, 1), IR(29762, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpName, false, false, IR(45, 2), IR(29778, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpMemberName, false, false, IR(47, 3), IR(29783, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpString, true, false, IR(50, 2), IR(29794, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpLine, false, false, IR(52, 3), IR(29801, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExtension, false, false, IR(11, 1), IR(29806, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInstImport, true, false, IR(50, 2), IR(29816, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpExtInst, true, true, IR(55, 4), IR(29876, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpMemoryModel, false, false, IR(59, 2), IR(29948, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpEntryPoint, false, false, IR(61, 4), IR(30022, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpExecutionMode, false, false, IR(65, 2), IR(30065, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpCapability, false, false, IR(67, 1), IR(30107, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpTypeVoid, true, false, IR(68, 1), IR(30118, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeBool, true, false, IR(68, 1), IR(30127, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeInt, true, false, IR(69, 3), IR(30136, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFloat, true, false, IR(72, 3), IR(30181, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeVector, true, false, IR(75, 3), IR(30191, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeMatrix, true, false, IR(75, 3), IR(30202, 11), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeImage, true, false, IR(78, 9), IR(30326, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampler, true, false, IR(68, 1), IR(30336, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeSampledImage, true, false, IR(87, 2), IR(30348, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeArray, true, false, IR(89, 3), IR(30365, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeRuntimeArray, true, false, IR(87, 2), IR(30375, 17), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStruct, true, false, IR(92, 2), IR(30392, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeOpaque, true, false, IR(50, 2), IR(30403, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePointer, true, false, IR(94, 3), IR(30445, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeFunction, true, false, IR(97, 3), IR(30457, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeEvent, true, false, IR(68, 1), IR(30470, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeDeviceEvent, true, false, IR(68, 1), IR(30480, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeReserveId, true, false, IR(68, 1), IR(30496, 14), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeQueue, true, false, IR(68, 1), IR(30510, 10), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypePipe, true, false, IR(100, 2), IR(30520, 9), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeForwardPointer, false, false, IR(102, 2), IR(30529, 19), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantTrue, true, true, IR(39, 2), IR(30548, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantFalse, true, true, IR(39, 2), IR(30561, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstant, true, true, IR(104, 3), IR(18202, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantComposite, true, true, IR(107, 3), IR(30613, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantSampler, true, true, IR(110, 5), IR(30709, 16), IR(0, 0), IR(225, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantNull, true, true, IR(39, 2), IR(30725, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantTrue, true, true, IR(39, 2), IR(30738, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantFalse, true, true, IR(39, 2), IR(30755, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstant, true, true, IR(104, 3), IR(30773, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantComposite, true, true, IR(107, 3), IR(30786, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantOp, true, true, IR(115, 3), IR(30849, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpFunction, true, true, IR(118, 4), IR(28855, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionParameter, true, true, IR(39, 2), IR(30898, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionEnd, false, false, IR(0, 0), IR(30916, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpFunctionCall, true, true, IR(122, 4), IR(30928, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpVariable, true, true, IR(126, 4), IR(30941, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpImageTexelPointer, true, true, IR(130, 5), IR(30950, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpLoad, true, true, IR(135, 4), IR(31008, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpStore, false, false, IR(139, 3), IR(31013, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemory, false, false, IR(142, 4), IR(31019, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCopyMemorySized, false, false, IR(146, 5), IR(31030, 16), IR(0, 0), IR(226, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAccessChain, true, true, IR(122, 4), IR(31046, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsAccessChain, true, true, IR(122, 4), IR(31058, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrAccessChain, true, true, IR(151, 5), IR(31078, 15), IR(0, 0), IR(228, 4), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArrayLength, true, true, IR(156, 4), IR(31093, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGenericPtrMemSemantics, true, true, IR(160, 3), IR(31105, 23), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpInBoundsPtrAccessChain, true, true, IR(151, 5), IR(31128, 23), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpDecorate, false, false, IR(163, 2), IR(31179, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorate, false, false, IR(165, 3), IR(31188, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpDecorationGroup, true, false, IR(68, 1), IR(31203, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupDecorate, false, false, IR(168, 2), IR(31219, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupMemberDecorate, false, false, IR(170, 2), IR(31278, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVectorExtractDynamic, true, true, IR(172, 4), IR(31298, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorInsertDynamic, true, true, IR(130, 5), IR(31319, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpVectorShuffle, true, true, IR(176, 5), IR(31339, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeConstruct, true, true, IR(107, 3), IR(31353, 19), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtract, true, true, IR(181, 4), IR(31372, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeInsert, true, true, IR(176, 5), IR(31389, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCopyObject, true, true, IR(160, 3), IR(31405, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTranspose, true, true, IR(160, 3), IR(31416, 10), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpSampledImage, true, true, IR(172, 4), IR(31426, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleImplicitLod, true, true, IR(185, 5), IR(31471, 23), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleExplicitLod, true, true, IR(190, 5), IR(31517, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefImplicitLod, true, true, IR(195, 6), IR(31540, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleDrefExplicitLod, true, true, IR(201, 6), IR(31567, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjImplicitLod, true, true, IR(185, 5), IR(31594, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjExplicitLod, true, true, IR(190, 5), IR(31621, 27), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(31648, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(31679, 31), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageFetch, true, true, IR(185, 5), IR(31710, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageGather, true, true, IR(195, 6), IR(31721, 12), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageDrefGather, true, true, IR(195, 6), IR(31733, 16), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageRead, true, true, IR(185, 5), IR(31749, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageWrite, false, false, IR(207, 4), IR(31759, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImage, true, true, IR(160, 3), IR(28899, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryFormat, true, true, IR(160, 3), IR(31770, 17), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryOrder, true, true, IR(160, 3), IR(31787, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySizeLod, true, true, IR(172, 4), IR(31803, 18), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySize, true, true, IR(160, 3), IR(31821, 15), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLod, true, true, IR(172, 4), IR(31836, 14), IR(0, 0), IR(234, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQueryLevels, true, true, IR(160, 3), IR(31850, 17), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageQuerySamples, true, true, IR(160, 3), IR(31867, 18), IR(0, 0), IR(232, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertFToU, true, true, IR(160, 3), IR(31885, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertFToS, true, true, IR(160, 3), IR(31897, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertSToF, true, true, IR(160, 3), IR(31909, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToF, true, true, IR(160, 3), IR(31921, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpUConvert, true, true, IR(160, 3), IR(31933, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSConvert, true, true, IR(160, 3), IR(31942, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpFConvert, true, true, IR(160, 3), IR(31951, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpQuantizeToF16, true, true, IR(160, 3), IR(31960, 14), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertPtrToU, true, true, IR(160, 3), IR(31974, 14), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertSToU, true, true, IR(160, 3), IR(31988, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSatConvertUToS, true, true, IR(160, 3), IR(32003, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToPtr, true, true, IR(160, 3), IR(32018, 14), IR(0, 0), IR(223, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpPtrCastToGeneric, true, true, IR(160, 3), IR(32032, 17), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtr, true, true, IR(160, 3), IR(32049, 17), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpGenericCastToPtrExplicit, true, true, IR(211, 4), IR(32066, 25), IR(0, 0), IR(217, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpBitcast, true, true, IR(160, 3), IR(32091, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpSNegate, true, true, IR(160, 3), IR(32099, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFNegate, true, true, IR(160, 3), IR(32107, 8), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAdd, true, true, IR(172, 4), IR(32115, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFAdd, true, true, IR(172, 4), IR(32120, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISub, true, true, IR(172, 4), IR(32125, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFSub, true, true, IR(172, 4), IR(32130, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIMul, true, true, IR(172, 4), IR(32135, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMul, true, true, IR(172, 4), IR(32140, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDiv, true, true, IR(172, 4), IR(32145, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDiv, true, true, IR(172, 4), IR(32150, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFDiv, true, true, IR(172, 4), IR(32155, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMod, true, true, IR(172, 4), IR(32160, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSRem, true, true, IR(172, 4), IR(32165, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMod, true, true, IR(172, 4), IR(32170, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFRem, true, true, IR(172, 4), IR(32175, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpFMod, true, true, IR(172, 4), IR(32180, 5), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesScalar, true, true, IR(172, 4), IR(32185, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesScalar, true, true, IR(172, 4), IR(32203, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpVectorTimesMatrix, true, true, IR(172, 4), IR(32221, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesVector, true, true, IR(172, 4), IR(32239, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpMatrixTimesMatrix, true, true, IR(172, 4), IR(32257, 18), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpOuterProduct, true, true, IR(172, 4), IR(32275, 13), IR(0, 0), IR(68, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpDot, true, true, IR(172, 4), IR(32288, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpIAddCarry, true, true, IR(172, 4), IR(32292, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpISubBorrow, true, true, IR(172, 4), IR(32302, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUMulExtended, true, true, IR(172, 4), IR(32313, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSMulExtended, true, true, IR(172, 4), IR(32326, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpAny, true, true, IR(160, 3), IR(32339, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpAll, true, true, IR(160, 3), IR(32343, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNan, true, true, IR(160, 3), IR(32347, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsInf, true, true, IR(160, 3), IR(32353, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsFinite, true, true, IR(160, 3), IR(32359, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIsNormal, true, true, IR(160, 3), IR(32368, 9), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSignBitSet, true, true, IR(160, 3), IR(32377, 11), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLessOrGreater, true, true, IR(172, 4), IR(32388, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,5), PrintingClass::kRelational_and_Logical},
  {spv::Op::OpOrdered, true, true, IR(172, 4), IR(32402, 8), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUnordered, true, true, IR(172, 4), IR(32410, 10), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalEqual, true, true, IR(172, 4), IR(32420, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNotEqual, true, true, IR(172, 4), IR(32433, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalOr, true, true, IR(172, 4), IR(32449, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalAnd, true, true, IR(172, 4), IR(32459, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpLogicalNot, true, true, IR(160, 3), IR(32470, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSelect, true, true, IR(130, 5), IR(32481, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpIEqual, true, true, IR(172, 4), IR(32488, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpINotEqual, true, true, IR(172, 4), IR(32495, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThan, true, true, IR(172, 4), IR(32505, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThan, true, true, IR(172, 4), IR(32518, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpUGreaterThanEqual, true, true, IR(172, 4), IR(32531, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSGreaterThanEqual, true, true, IR(172, 4), IR(32549, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThan, true, true, IR(172, 4), IR(32567, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThan, true, true, IR(172, 4), IR(32577, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpULessThanEqual, true, true, IR(172, 4), IR(32587, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpSLessThanEqual, true, true, IR(172, 4), IR(32602, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdEqual, true, true, IR(172, 4), IR(32617, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordEqual, true, true, IR(172, 4), IR(32627, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdNotEqual, true, true, IR(172, 4), IR(32639, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordNotEqual, true, true, IR(172, 4), IR(32652, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThan, true, true, IR(172, 4), IR(32667, 13), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThan, true, true, IR(172, 4), IR(32680, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThan, true, true, IR(172, 4), IR(32695, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThan, true, true, IR(172, 4), IR(32711, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdLessThanEqual, true, true, IR(172, 4), IR(32729, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordLessThanEqual, true, true, IR(172, 4), IR(32747, 20), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFOrdGreaterThanEqual, true, true, IR(172, 4), IR(32767, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpFUnordGreaterThanEqual, true, true, IR(172, 4), IR(32788, 23), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kRelational_and_Logical},
  {spv::Op::OpShiftRightLogical, true, true, IR(172, 4), IR(32811, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftRightArithmetic, true, true, IR(172, 4), IR(32829, 21), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpShiftLeftLogical, true, true, IR(172, 4), IR(32850, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseOr, true, true, IR(172, 4), IR(32867, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseXor, true, true, IR(172, 4), IR(32877, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitwiseAnd, true, true, IR(172, 4), IR(32888, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpNot, true, true, IR(160, 3), IR(32899, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldInsert, true, true, IR(215, 6), IR(32903, 15), IR(0, 0), IR(235, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldSExtract, true, true, IR(130, 5), IR(32918, 17), IR(0, 0), IR(235, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitFieldUExtract, true, true, IR(130, 5), IR(32935, 17), IR(0, 0), IR(235, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitReverse, true, true, IR(160, 3), IR(32952, 11), IR(0, 0), IR(235, 2), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpBitCount, true, true, IR(160, 3), IR(32963, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpDPdx, true, true, IR(160, 3), IR(32972, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdy, true, true, IR(160, 3), IR(32977, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidth, true, true, IR(160, 3), IR(32982, 7), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxFine, true, true, IR(160, 3), IR(32989, 9), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyFine, true, true, IR(160, 3), IR(32998, 9), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthFine, true, true, IR(160, 3), IR(33007, 11), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdxCoarse, true, true, IR(160, 3), IR(33018, 11), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpDPdyCoarse, true, true, IR(160, 3), IR(33029, 11), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpFwidthCoarse, true, true, IR(160, 3), IR(33040, 13), IR(0, 0), IR(237, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDerivative},
  {spv::Op::OpEmitVertex, false, false, IR(0, 0), IR(33053, 11), IR(0, 0), IR(71, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndPrimitive, false, false, IR(0, 0), IR(33064, 13), IR(0, 0), IR(71, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEmitStreamVertex, false, false, IR(10, 1), IR(33077, 17), IR(0, 0), IR(124, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpEndStreamPrimitive, false, false, IR(10, 1), IR(33094, 19), IR(0, 0), IR(124, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPrimitive},
  {spv::Op::OpControlBarrier, false, false, IR(221, 3), IR(33150, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryBarrier, false, false, IR(224, 2), IR(33165, 14), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpAtomicLoad, true, true, IR(226, 5), IR(33179, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicStore, false, false, IR(231, 4), IR(33190, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicExchange, true, true, IR(235, 6), IR(33202, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchange, true, true, IR(241, 8), IR(33217, 22), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicCompareExchangeWeak, true, true, IR(241, 8), IR(33239, 26), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), SPV_SPIRV_VERSION_WORD(1,3), PrintingClass::kAtomic},
  {spv::Op::OpAtomicIIncrement, true, true, IR(226, 5), IR(33265, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIDecrement, true, true, IR(226, 5), IR(33282, 17), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicIAdd, true, true, IR(235, 6), IR(33299, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicISub, true, true, IR(235, 6), IR(33310, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMin, true, true, IR(235, 6), IR(33321, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMin, true, true, IR(235, 6), IR(33332, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicSMax, true, true, IR(235, 6), IR(33343, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicUMax, true, true, IR(235, 6), IR(33354, 11), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicAnd, true, true, IR(235, 6), IR(33365, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicOr, true, true, IR(235, 6), IR(33375, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicXor, true, true, IR(235, 6), IR(33384, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpPhi, true, true, IR(107, 3), IR(33394, 4), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLoopMerge, false, false, IR(249, 3), IR(33428, 10), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSelectionMerge, false, false, IR(252, 2), IR(33473, 15), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLabel, true, false, IR(68, 1), IR(33488, 6), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranch, false, false, IR(10, 1), IR(33494, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpBranchConditional, false, false, IR(254, 4), IR(33501, 18), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpSwitch, false, false, IR(258, 3), IR(33564, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpKill, false, false, IR(0, 0), IR(33571, 5), IR(0, 0), IR(3, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturn, false, false, IR(0, 0), IR(33576, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpReturnValue, false, false, IR(10, 1), IR(33583, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUnreachable, false, false, IR(0, 0), IR(33595, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStart, false, false, IR(261, 2), IR(33607, 14), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpLifetimeStop, false, false, IR(261, 2), IR(33621, 13), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpGroupAsyncCopy, true, true, IR(263, 8), IR(33634, 15), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupWaitEvents, false, false, IR(271, 3), IR(33649, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAll, true, true, IR(274, 4), IR(33665, 9), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupAny, true, true, IR(274, 4), IR(33674, 9), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBroadcast, true, true, IR(278, 5), IR(33683, 15), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupIAdd, true, true, IR(283, 5), IR(33731, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAdd, true, true, IR(283, 5), IR(33741, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMin, true, true, IR(283, 5), IR(33751, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMin, true, true, IR(283, 5), IR(33761, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMin, true, true, IR(283, 5), IR(33771, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMax, true, true, IR(283, 5), IR(33781, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMax, true, true, IR(283, 5), IR(33791, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMax, true, true, IR(283, 5), IR(33801, 10), IR(0, 0), IR(238, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpReadPipe, true, true, IR(215, 6), IR(33811, 9), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpWritePipe, true, true, IR(215, 6), IR(33820, 10), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedReadPipe, true, true, IR(288, 8), IR(33830, 17), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReservedWritePipe, true, true, IR(288, 8), IR(33847, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveReadPipePackets, true, true, IR(215, 6), IR(33865, 23), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpReserveWritePipePackets, true, true, IR(215, 6), IR(33888, 24), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitReadPipe, false, false, IR(296, 4), IR(33912, 15), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCommitWritePipe, false, false, IR(296, 4), IR(33927, 16), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpIsValidReserveId, true, true, IR(160, 3), IR(33943, 17), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetNumPipePackets, true, true, IR(130, 5), IR(33960, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetMaxPipePackets, true, true, IR(130, 5), IR(33978, 18), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveReadPipePackets, true, true, IR(300, 7), IR(33996, 28), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupReserveWritePipePackets, true, true, IR(300, 7), IR(34024, 29), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitReadPipe, false, false, IR(307, 5), IR(34053, 20), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGroupCommitWritePipe, false, false, IR(307, 5), IR(34073, 21), IR(0, 0), IR(77, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpEnqueueMarker, true, true, IR(215, 6), IR(34094, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpEnqueueKernel, true, true, IR(312, 13), IR(34108, 14), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeSubGroupCount, true, true, IR(325, 7), IR(34122, 30), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelNDrangeMaxSubGroupSize, true, true, IR(325, 7), IR(34152, 32), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelWorkGroupSize, true, true, IR(215, 6), IR(34184, 23), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelPreferredWorkGroupSizeMultiple, true, true, IR(215, 6), IR(34207, 40), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpRetainEvent, false, false, IR(10, 1), IR(34247, 12), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpReleaseEvent, false, false, IR(10, 1), IR(34259, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCreateUserEvent, true, true, IR(39, 2), IR(34272, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpIsValidEvent, true, true, IR(160, 3), IR(34288, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpSetUserEventStatus, false, false, IR(36, 2), IR(34301, 19), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpCaptureEventProfilingInfo, false, false, IR(33, 3), IR(34320, 26), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetDefaultQueue, true, true, IR(39, 2), IR(34346, 16), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpBuildNDRange, true, true, IR(130, 5), IR(34362, 13), IR(0, 0), IR(76, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpImageSparseSampleImplicitLod, true, true, IR(185, 5), IR(34375, 29), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleExplicitLod, true, true, IR(190, 5), IR(34404, 29), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefImplicitLod, true, true, IR(195, 6), IR(34433, 33), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleDrefExplicitLod, true, true, IR(201, 6), IR(34466, 33), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjImplicitLod, true, true, IR(185, 5), IR(34499, 33), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjExplicitLod, true, true, IR(190, 5), IR(34532, 33), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefImplicitLod, true, true, IR(195, 6), IR(34565, 37), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseSampleProjDrefExplicitLod, true, true, IR(201, 6), IR(34602, 37), IR(0, 0), IR(239, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseFetch, true, true, IR(185, 5), IR(34639, 17), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseGather, true, true, IR(195, 6), IR(34656, 18), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseDrefGather, true, true, IR(195, 6), IR(34674, 22), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageSparseTexelsResident, true, true, IR(160, 3), IR(34696, 26), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpNoLine, false, false, IR(0, 0), IR(34722, 7), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpAtomicFlagTestAndSet, true, true, IR(226, 5), IR(34729, 21), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFlagClear, false, false, IR(332, 3), IR(34750, 16), IR(0, 0), IR(0, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpImageSparseRead, true, true, IR(185, 5), IR(34766, 16), IR(0, 0), IR(239, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,0), 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpSizeOf, true, true, IR(160, 3), IR(34782, 7), IR(0, 0), IR(1, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTypePipeStorage, true, false, IR(68, 1), IR(34789, 16), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantPipeStorage, true, true, IR(335, 5), IR(34805, 20), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpCreatePipeFromPipeStorage, true, true, IR(160, 3), IR(34825, 26), IR(0, 0), IR(240, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kPipe},
  {spv::Op::OpGetKernelLocalSizeForSubgroupCount, true, true, IR(325, 7), IR(34851, 35), IR(0, 0), IR(166, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpGetKernelMaxNumSubgroups, true, true, IR(215, 6), IR(34886, 25), IR(0, 0), IR(166, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDevice_Side_Enqueue},
  {spv::Op::OpTypeNamedBarrier, true, false, IR(68, 1), IR(34911, 17), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpNamedBarrierInitialize, true, true, IR(160, 3), IR(34928, 23), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpMemoryNamedBarrier, false, false, IR(332, 3), IR(34951, 19), IR(0, 0), IR(241, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpModuleProcessed, false, false, IR(11, 1), IR(34970, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,1), 0xffffffffu, PrintingClass::kDebug},
  {spv::Op::OpExecutionModeId, false, false, IR(65, 2), IR(34986, 16), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpDecorateId, false, false, IR(163, 2), IR(35002, 11), IR(0, 0), IR(1, 0), IR(205, 1), SPV_SPIRV_VERSION_WORD(1,2), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpGroupNonUniformElect, true, true, IR(340, 3), IR(35013, 21), IR(0, 0), IR(78, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAll, true, true, IR(274, 4), IR(35034, 19), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAny, true, true, IR(274, 4), IR(35053, 19), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformAllEqual, true, true, IR(274, 4), IR(35072, 24), IR(0, 0), IR(242, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcast, true, true, IR(278, 5), IR(35096, 25), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBroadcastFirst, true, true, IR(274, 4), IR(35121, 30), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallot, true, true, IR(274, 4), IR(6960, 22), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformInverseBallot, true, true, IR(274, 4), IR(35151, 29), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitExtract, true, true, IR(278, 5), IR(35180, 32), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotBitCount, true, true, IR(283, 5), IR(35212, 30), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindLSB, true, true, IR(274, 4), IR(35242, 29), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBallotFindMSB, true, true, IR(274, 4), IR(35271, 29), IR(0, 0), IR(243, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffle, true, true, IR(278, 5), IR(10142, 23), IR(0, 0), IR(244, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleXor, true, true, IR(278, 5), IR(35300, 26), IR(0, 0), IR(244, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleUp, true, true, IR(278, 5), IR(35326, 25), IR(0, 0), IR(245, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformShuffleDown, true, true, IR(278, 5), IR(35351, 27), IR(0, 0), IR(245, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIAdd, true, true, IR(343, 6), IR(35378, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFAdd, true, true, IR(343, 6), IR(35398, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformIMul, true, true, IR(343, 6), IR(35418, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMul, true, true, IR(343, 6), IR(35438, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMin, true, true, IR(343, 6), IR(35458, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMin, true, true, IR(343, 6), IR(35478, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMin, true, true, IR(343, 6), IR(35498, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformSMax, true, true, IR(343, 6), IR(35518, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformUMax, true, true, IR(343, 6), IR(35538, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformFMax, true, true, IR(343, 6), IR(35558, 20), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseAnd, true, true, IR(343, 6), IR(35578, 26), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseOr, true, true, IR(343, 6), IR(35604, 25), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformBitwiseXor, true, true, IR(343, 6), IR(35629, 26), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalAnd, true, true, IR(343, 6), IR(35655, 26), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalOr, true, true, IR(343, 6), IR(35681, 25), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformLogicalXor, true, true, IR(343, 6), IR(35706, 26), IR(0, 0), IR(246, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadBroadcast, true, true, IR(278, 5), IR(35732, 29), IR(0, 0), IR(249, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadSwap, true, true, IR(278, 5), IR(35761, 24), IR(0, 0), IR(249, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,3), 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpCopyLogical, true, true, IR(160, 3), IR(35785, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPtrEqual, true, true, IR(172, 4), IR(35797, 9), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrNotEqual, true, true, IR(172, 4), IR(35806, 12), IR(0, 0), IR(1, 0), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPtrDiff, true, true, IR(172, 4), IR(35818, 8), IR(0, 0), IR(250, 3), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpColorAttachmentReadEXT, true, true, IR(349, 4), IR(35826, 23), IR(0, 0), IR(158, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpDepthAttachmentReadEXT, true, true, IR(353, 3), IR(35849, 23), IR(0, 0), IR(167, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpStencilAttachmentReadEXT, true, true, IR(353, 3), IR(35872, 25), IR(0, 0), IR(168, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeTensorARM, true, false, IR(356, 4), IR(35897, 14), IR(0, 0), IR(222, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTensorReadARM, true, true, IR(360, 5), IR(35953, 14), IR(0, 0), IR(222, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorWriteARM, false, false, IR(365, 4), IR(35967, 15), IR(0, 0), IR(222, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpTensorQuerySizeARM, true, true, IR(172, 4), IR(35982, 19), IR(0, 0), IR(222, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kTensor},
  {spv::Op::OpGraphConstantARM, true, true, IR(369, 3), IR(36001, 17), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEntryPointARM, false, false, IR(372, 3), IR(36018, 19), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphARM, true, true, IR(39, 2), IR(10433, 9), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphInputARM, true, true, IR(122, 4), IR(36037, 14), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphSetOutputARM, false, false, IR(375, 3), IR(36051, 18), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpGraphEndARM, false, false, IR(0, 0), IR(36069, 12), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGraph},
  {spv::Op::OpTypeGraphARM, true, false, IR(378, 3), IR(36081, 13), IR(0, 0), IR(253, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBitcastExtractEXT, true, true, IR(172, 4), IR(10580, 18), IR(0, 0), IR(254, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpTerminateInvocation, false, false, IR(0, 0), IR(36094, 20), IR(0, 0), IR(3, 1), IR(215, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpTypeUntypedPointerKHR, true, false, IR(381, 2), IR(36114, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpUntypedVariableKHR, true, true, IR(383, 5), IR(36136, 19), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedAccessChainKHR, true, true, IR(151, 5), IR(36155, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsAccessChainKHR, true, true, IR(151, 5), IR(36177, 30), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSubgroupBallotKHR, true, true, IR(160, 3), IR(6721, 18), IR(0, 0), IR(255, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupFirstInvocationKHR, true, true, IR(160, 3), IR(36207, 27), IR(0, 0), IR(255, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUntypedPtrAccessChainKHR, true, true, IR(388, 6), IR(36234, 25), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedInBoundsPtrAccessChainKHR, true, true, IR(388, 6), IR(36259, 33), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedArrayLengthKHR, true, true, IR(394, 5), IR(36292, 22), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpUntypedPrefetchKHR, false, false, IR(399, 5), IR(36314, 19), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpFmaKHR, true, true, IR(130, 5), IR(36333, 7), IR(0, 0), IR(256, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSubgroupAllKHR, true, true, IR(160, 3), IR(36340, 15), IR(0, 0), IR(257, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAnyKHR, true, true, IR(160, 3), IR(36355, 15), IR(0, 0), IR(257, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupAllEqualKHR, true, true, IR(160, 3), IR(36370, 20), IR(0, 0), IR(257, 1), IR(42, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupNonUniformRotateKHR, true, true, IR(404, 6), IR(15614, 25), IR(0, 0), IR(258, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupReadInvocationKHR, true, true, IR(172, 4), IR(36390, 26), IR(0, 0), IR(255, 1), IR(2, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpExtInstWithForwardRefsKHR, true, true, IR(410, 5), IR(36416, 26), IR(0, 0), IR(1, 0), IR(216, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpUntypedGroupAsyncCopyKHR, true, true, IR(415, 11), IR(36442, 25), IR(0, 0), IR(94, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpTraceRayKHR, false, false, IR(426, 11), IR(36467, 12), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpExecuteCallableKHR, false, false, IR(36, 2), IR(36479, 19), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToAccelerationStructureKHR, true, true, IR(160, 3), IR(36498, 35), IR(0, 0), IR(259, 2), IR(217, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionKHR, false, false, IR(0, 0), IR(36533, 22), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayKHR, false, false, IR(0, 0), IR(36555, 16), IR(0, 0), IR(61, 1), IR(28, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSDot, true, true, IR(437, 5), IR(36618, 5), IR(176, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDot, true, true, IR(437, 5), IR(36631, 5), IR(177, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDot, true, true, IR(437, 5), IR(36644, 6), IR(178, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSDotAccSat, true, true, IR(442, 6), IR(36659, 11), IR(179, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpUDotAccSat, true, true, IR(442, 6), IR(36684, 11), IR(180, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpSUDotAccSat, true, true, IR(442, 6), IR(36709, 12), IR(181, 1), IR(261, 1), IR(158, 1), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpTypeCooperativeMatrixKHR, true, false, IR(448, 6), IR(36736, 25), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadKHR, true, true, IR(454, 6), IR(36761, 25), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreKHR, false, false, IR(460, 5), IR(36786, 26), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixMulAddKHR, true, true, IR(465, 6), IR(36866, 27), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCooperativeMatrixLengthKHR, true, true, IR(160, 3), IR(36893, 27), IR(0, 0), IR(88, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(36920, 30), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeReplicateEXT, true, true, IR(160, 3), IR(36950, 34), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructReplicateEXT, true, true, IR(160, 3), IR(36984, 31), IR(0, 0), IR(262, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpTypeRayQueryKHR, true, false, IR(68, 1), IR(37015, 16), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpRayQueryInitializeKHR, false, false, IR(471, 8), IR(37031, 22), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryTerminateKHR, false, false, IR(10, 1), IR(37053, 21), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGenerateIntersectionKHR, false, false, IR(36, 2), IR(37074, 32), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryConfirmIntersectionKHR, false, false, IR(10, 1), IR(37106, 31), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryProceedKHR, true, true, IR(160, 3), IR(37137, 19), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTypeKHR, true, true, IR(172, 4), IR(37156, 31), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpImageSampleWeightedQCOM, true, true, IR(130, 5), IR(37187, 24), IR(0, 0), IR(263, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBoxFilterQCOM, true, true, IR(130, 5), IR(37211, 19), IR(0, 0), IR(264, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSSDQCOM, true, true, IR(325, 7), IR(37230, 23), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchSADQCOM, true, true, IR(325, 7), IR(37253, 23), IR(0, 0), IR(265, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBitCastArrayQCOM, true, true, IR(160, 3), IR(37276, 17), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpImageBlockMatchWindowSSDQCOM, true, true, IR(325, 7), IR(37293, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchWindowSADQCOM, true, true, IR(325, 7), IR(37322, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSSDQCOM, true, true, IR(325, 7), IR(37351, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpImageBlockMatchGatherSADQCOM, true, true, IR(325, 7), IR(37380, 29), IR(0, 0), IR(267, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpBFloat16MulAddQCOM, true, true, IR(130, 5), IR(11353, 19), IR(0, 0), IR(268, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kArithmetic},
  {spv::Op::OpCompositeConstructCoopMatQCOM, true, true, IR(160, 3), IR(37409, 30), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpCompositeExtractCoopMatQCOM, true, true, IR(160, 3), IR(37439, 28), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpExtractSubArrayQCOM, true, true, IR(172, 4), IR(37467, 20), IR(0, 0), IR(266, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpImageGatherQCOM, true, true, IR(479, 7), IR(37487, 16), IR(0, 0), IR(269, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpGroupIAddNonUniformAMD, true, true, IR(283, 5), IR(37503, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFAddNonUniformAMD, true, true, IR(283, 5), IR(37526, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMinNonUniformAMD, true, true, IR(283, 5), IR(37549, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMinNonUniformAMD, true, true, IR(283, 5), IR(37572, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMinNonUniformAMD, true, true, IR(283, 5), IR(37595, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMaxNonUniformAMD, true, true, IR(283, 5), IR(37618, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupUMaxNonUniformAMD, true, true, IR(283, 5), IR(37641, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupSMaxNonUniformAMD, true, true, IR(283, 5), IR(37664, 23), IR(0, 0), IR(238, 1), IR(33, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpFragmentMaskFetchAMD, true, true, IR(172, 4), IR(37687, 21), IR(0, 0), IR(271, 1), IR(63, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFragmentFetchAMD, true, true, IR(130, 5), IR(37708, 17), IR(0, 0), IR(271, 1), IR(63, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReadClockKHR, true, true, IR(340, 3), IR(37725, 13), IR(0, 0), IR(272, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAllocateNodePayloadsAMDX, true, true, IR(278, 5), IR(37738, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEnqueueNodePayloadsAMDX, false, false, IR(10, 1), IR(37763, 24), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeNodePayloadArrayAMDX, true, false, IR(87, 2), IR(37787, 25), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFinishWritingNodePayloadAMDX, true, true, IR(160, 3), IR(37812, 29), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpNodePayloadArrayLengthAMDX, true, true, IR(160, 3), IR(37841, 27), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIsNodePayloadValidAMDX, true, true, IR(172, 4), IR(37868, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantStringAMDX, true, false, IR(50, 2), IR(37891, 19), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSpecConstantStringAMDX, true, false, IR(50, 2), IR(37910, 23), IR(0, 0), IR(43, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformQuadAllKHR, true, true, IR(160, 3), IR(37933, 26), IR(0, 0), IR(176, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpGroupNonUniformQuadAnyKHR, true, true, IR(160, 3), IR(37959, 26), IR(0, 0), IR(176, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpTypeBufferEXT, true, false, IR(381, 2), IR(37985, 14), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpBufferPointerEXT, true, true, IR(160, 3), IR(37999, 17), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpAbortKHR, false, false, IR(36, 2), IR(11690, 9), IR(0, 0), IR(273, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpUntypedImageTexelPointerEXT, true, true, IR(215, 6), IR(38016, 28), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMemberDecorateIdEXT, false, false, IR(165, 3), IR(38044, 20), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpConstantSizeOfEXT, true, true, IR(160, 3), IR(38064, 18), IR(0, 0), IR(44, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConstantDataKHR, true, true, IR(486, 3), IR(11699, 16), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantDataKHR, true, true, IR(486, 3), IR(38082, 20), IR(0, 0), IR(128, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpPoisonKHR, true, true, IR(39, 2), IR(38102, 10), IR(0, 0), IR(177, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpFreezeKHR, true, true, IR(160, 3), IR(38112, 10), IR(0, 0), IR(177, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpHitObjectRecordHitMotionNV, false, false, IR(489, 14), IR(38122, 27), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexMotionNV, false, false, IR(503, 13), IR(38149, 36), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionNV, false, false, IR(516, 7), IR(38185, 28), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectNV, true, true, IR(160, 3), IR(38213, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldNV, true, true, IR(160, 3), IR(38241, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionNV, true, true, IR(160, 3), IR(38269, 33), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginNV, true, true, IR(160, 3), IR(38302, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionNV, false, false, IR(503, 13), IR(38332, 26), IR(0, 0), IR(274, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleNV, true, true, IR(160, 3), IR(38358, 39), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexNV, true, true, IR(160, 3), IR(38397, 44), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyNV, false, false, IR(10, 1), IR(38441, 23), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayNV, false, false, IR(523, 12), IR(38464, 20), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitNV, false, false, IR(503, 13), IR(38484, 21), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordHitWithIndexNV, false, false, IR(523, 12), IR(38505, 30), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissNV, false, false, IR(535, 6), IR(38535, 22), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderNV, false, false, IR(36, 2), IR(38557, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeNV, true, true, IR(160, 3), IR(38582, 26), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesNV, false, false, IR(36, 2), IR(38608, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindNV, true, true, IR(160, 3), IR(38633, 22), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexNV, true, true, IR(160, 3), IR(38655, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexNV, true, true, IR(160, 3), IR(38684, 28), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdNV, true, true, IR(160, 3), IR(38712, 25), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexNV, true, true, IR(160, 3), IR(38737, 34), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionNV, true, true, IR(160, 3), IR(38771, 32), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginNV, true, true, IR(160, 3), IR(38803, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxNV, true, true, IR(160, 3), IR(38832, 22), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinNV, true, true, IR(160, 3), IR(38854, 22), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyNV, true, true, IR(160, 3), IR(38876, 19), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitNV, true, true, IR(160, 3), IR(38895, 17), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissNV, true, true, IR(160, 3), IR(38912, 18), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectNV, false, false, IR(541, 3), IR(38930, 29), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHintNV, false, false, IR(36, 2), IR(38959, 24), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectNV, true, false, IR(68, 1), IR(38983, 16), IR(0, 0), IR(134, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpImageSampleFootprintNV, true, true, IR(479, 7), IR(38999, 23), IR(0, 0), IR(276, 1), IR(82, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpTypeVectorIdEXT, true, false, IR(89, 3), IR(39022, 16), IR(182, 1), IR(277, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeVectorMatrixMulNV, true, true, IR(544, 13), IR(39062, 29), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorOuterProductAccumulateNV, false, false, IR(557, 7), IR(39091, 42), IR(0, 0), IR(280, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorReduceSumAccumulateNV, false, false, IR(33, 3), IR(39133, 39), IR(0, 0), IR(280, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorMatrixMulAddNV, true, true, IR(564, 16), IR(39172, 32), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixConvertUseEXT, true, true, IR(160, 3), IR(39204, 31), IR(183, 1), IR(281, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpEmitMeshTasksEXT, false, false, IR(580, 4), IR(39262, 17), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSetMeshOutputsEXT, false, false, IR(36, 2), IR(39279, 18), IR(0, 0), IR(54, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpGroupNonUniformPartitionEXT, true, true, IR(160, 3), IR(39297, 28), IR(184, 1), IR(202, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kNon_Uniform},
  {spv::Op::OpWritePackedPrimitiveIndices4x8NV, false, false, IR(36, 2), IR(39352, 33), IR(0, 0), IR(51, 1), IR(18, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexPositionNV, true, true, IR(325, 7), IR(39385, 35), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFetchMicroTriangleVertexBarycentricNV, true, true, IR(325, 7), IR(39420, 38), IR(0, 0), IR(283, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeVectorLoadNV, true, true, IR(584, 5), IR(39458, 24), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeVectorStoreNV, false, false, IR(589, 4), IR(39482, 25), IR(0, 0), IR(279, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpHitObjectRecordFromQueryEXT, false, false, IR(593, 5), IR(39507, 28), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissEXT, false, false, IR(516, 7), IR(39535, 23), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordMissMotionEXT, false, false, IR(471, 8), IR(39558, 29), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetIntersectionTriangleVertexPositionsEXT, true, true, IR(160, 3), IR(39587, 51), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayFlagsEXT, true, true, IR(160, 3), IR(39638, 24), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectSetShaderBindingTableRecordIndexEXT, false, false, IR(36, 2), IR(39662, 45), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectReorderExecuteShaderEXT, false, false, IR(598, 4), IR(39707, 33), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceReorderExecuteEXT, false, false, IR(602, 14), IR(39740, 32), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceMotionReorderExecuteEXT, false, false, IR(616, 15), IR(39772, 38), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeHitObjectEXT, true, false, IR(68, 1), IR(39810, 17), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpReorderThreadWithHintEXT, false, false, IR(36, 2), IR(39827, 25), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReorderThreadWithHitObjectEXT, false, false, IR(541, 3), IR(39852, 30), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayEXT, false, false, IR(523, 12), IR(39882, 21), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectTraceRayMotionEXT, false, false, IR(503, 13), IR(39903, 27), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectRecordEmptyEXT, false, false, IR(10, 1), IR(39930, 24), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectExecuteShaderEXT, false, false, IR(36, 2), IR(39954, 26), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetCurrentTimeEXT, true, true, IR(160, 3), IR(39980, 27), IR(0, 0), IR(284, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetAttributesEXT, false, false, IR(36, 2), IR(40007, 26), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetHitKindEXT, true, true, IR(160, 3), IR(40033, 23), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetPrimitiveIndexEXT, true, true, IR(160, 3), IR(40056, 30), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetGeometryIndexEXT, true, true, IR(160, 3), IR(40086, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceIdEXT, true, true, IR(160, 3), IR(40115, 26), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetInstanceCustomIndexEXT, true, true, IR(160, 3), IR(40141, 35), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayOriginEXT, true, true, IR(160, 3), IR(40176, 31), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectRayDirectionEXT, true, true, IR(160, 3), IR(40207, 34), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayDirectionEXT, true, true, IR(160, 3), IR(40241, 33), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldRayOriginEXT, true, true, IR(160, 3), IR(40274, 30), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetObjectToWorldEXT, true, true, IR(160, 3), IR(40304, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetWorldToObjectEXT, true, true, IR(160, 3), IR(40333, 29), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMaxEXT, true, true, IR(160, 3), IR(40362, 23), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpReportIntersectionKHR, true, true, IR(172, 4), IR(40385, 22), IR(185, 1), IR(55, 2), IR(24, 2), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIgnoreIntersectionNV, false, false, IR(0, 0), IR(40428, 21), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTerminateRayNV, false, false, IR(0, 0), IR(40449, 15), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceNV, false, false, IR(426, 11), IR(40464, 8), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceMotionNV, false, false, IR(523, 12), IR(40472, 14), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTraceRayMotionNV, false, false, IR(523, 12), IR(40486, 17), IR(0, 0), IR(58, 1), IR(27, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTriangleVertexPositionsKHR, true, true, IR(172, 4), IR(40503, 50), IR(0, 0), IR(286, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeAccelerationStructureKHR, true, false, IR(68, 1), IR(40553, 29), IR(186, 1), IR(287, 4), IR(219, 4), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpExecuteCallableNV, false, false, IR(36, 2), IR(40610, 18), IR(0, 0), IR(57, 1), IR(26, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionClusterIdNV, true, true, IR(172, 4), IR(40628, 35), IR(187, 1), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetClusterIdNV, true, true, IR(160, 3), IR(40686, 24), IR(0, 0), IR(65, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetRayTMinEXT, true, true, IR(160, 3), IR(40710, 23), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderBindingTableRecordIndexEXT, true, true, IR(160, 3), IR(40733, 45), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetShaderRecordBufferHandleEXT, true, true, IR(160, 3), IR(40778, 40), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsEmptyEXT, true, true, IR(160, 3), IR(40818, 20), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsHitEXT, true, true, IR(160, 3), IR(40838, 18), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsMissEXT, true, true, IR(160, 3), IR(40856, 19), IR(0, 0), IR(135, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTypeCooperativeMatrixNV, true, false, IR(631, 5), IR(40875, 24), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCooperativeMatrixLoadNV, true, true, IR(636, 6), IR(40899, 24), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixStoreNV, false, false, IR(642, 5), IR(40923, 25), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixMulAddNV, true, true, IR(130, 5), IR(40948, 26), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixLengthNV, true, true, IR(160, 3), IR(40974, 26), IR(0, 0), IR(291, 1), IR(90, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixGetCoordinateEXT, true, true, IR(172, 4), IR(13876, 34), IR(0, 0), IR(292, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpBeginInvocationInterlockEXT, false, false, IR(0, 0), IR(41000, 28), IR(0, 0), IR(293, 3), IR(91, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpEndInvocationInterlockEXT, false, false, IR(0, 0), IR(41028, 26), IR(0, 0), IR(293, 3), IR(91, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixReduceEXT, true, true, IR(647, 5), IR(41097, 27), IR(188, 1), IR(296, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpCooperativeMatrixLoadTensorNV, true, true, IR(652, 7), IR(41225, 30), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixStoreTensorNV, false, false, IR(659, 5), IR(41255, 31), IR(0, 0), IR(220, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpCooperativeMatrixPerElementOpEXT, true, true, IR(151, 5), IR(41286, 33), IR(189, 1), IR(297, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kFunction},
  {spv::Op::OpTypeTensorLayoutNV, true, false, IR(89, 3), IR(41351, 19), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeTensorViewNV, true, false, IR(664, 4), IR(41370, 17), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpCreateTensorLayoutNV, true, true, IR(39, 2), IR(41387, 21), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetDimensionNV, true, true, IR(122, 4), IR(41408, 27), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetStrideNV, true, true, IR(122, 4), IR(41435, 24), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSliceNV, true, true, IR(122, 4), IR(41459, 20), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetClampValueNV, true, true, IR(172, 4), IR(41479, 28), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCreateTensorViewNV, true, true, IR(39, 2), IR(41507, 19), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetDimensionNV, true, true, IR(122, 4), IR(41526, 25), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetStrideNV, true, true, IR(122, 4), IR(41551, 22), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpDemoteToHelperInvocation, false, false, IR(0, 0), IR(13230, 25), IR(46, 1), IR(299, 1), IR(0, 0), SPV_SPIRV_VERSION_WORD(1,6), 0xffffffffu, PrintingClass::kControl_Flow},
  {spv::Op::OpIsHelperInvocationEXT, true, true, IR(39, 2), IR(41573, 22), IR(0, 0), IR(299, 1), IR(92, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorViewSetClipNV, true, true, IR(325, 7), IR(41595, 20), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpTensorLayoutSetBlockSizeNV, true, true, IR(122, 4), IR(41615, 27), IR(0, 0), IR(298, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpCooperativeMatrixTransposeNV, true, true, IR(160, 3), IR(41642, 29), IR(0, 0), IR(300, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertUToImageNV, true, true, IR(160, 3), IR(41671, 18), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSamplerNV, true, true, IR(160, 3), IR(41689, 20), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertImageToUNV, true, true, IR(160, 3), IR(41709, 18), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSamplerToUNV, true, true, IR(160, 3), IR(41727, 20), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertUToSampledImageNV, true, true, IR(160, 3), IR(41747, 25), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConvertSampledImageToUNV, true, true, IR(160, 3), IR(41772, 25), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSamplerImageAddressingModeNV, false, false, IR(0, 1), IR(41797, 29), IR(0, 0), IR(136, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRawAccessChainNV, true, true, IR(668, 7), IR(41878, 17), IR(0, 0), IR(209, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRayQueryGetIntersectionSpherePositionNV, true, true, IR(172, 4), IR(41895, 40), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionSphereRadiusNV, true, true, IR(172, 4), IR(41935, 38), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSPositionsNV, true, true, IR(172, 4), IR(41973, 38), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSRadiiNV, true, true, IR(172, 4), IR(42011, 34), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionLSSHitValueNV, true, true, IR(172, 4), IR(42045, 37), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSpherePositionNV, true, true, IR(160, 3), IR(42082, 29), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetSphereRadiusNV, true, true, IR(160, 3), IR(42111, 27), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSPositionsNV, true, true, IR(160, 3), IR(42138, 27), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectGetLSSRadiiNV, true, true, IR(160, 3), IR(42165, 23), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsSphereHitNV, true, true, IR(160, 3), IR(42188, 23), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpHitObjectIsLSSHitNV, true, true, IR(160, 3), IR(42211, 20), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsSphereHitNV, true, true, IR(172, 4), IR(42231, 22), IR(0, 0), IR(62, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryIsLSSHitNV, true, true, IR(172, 4), IR(42253, 19), IR(0, 0), IR(63, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpSubgroupShuffleINTEL, true, true, IR(172, 4), IR(13961, 21), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleDownINTEL, true, true, IR(130, 5), IR(42272, 25), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleUpINTEL, true, true, IR(130, 5), IR(42297, 23), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupShuffleXorINTEL, true, true, IR(172, 4), IR(42320, 24), IR(0, 0), IR(301, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockReadINTEL, true, true, IR(160, 3), IR(42344, 23), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupBlockWriteINTEL, false, false, IR(36, 2), IR(42367, 24), IR(0, 0), IR(302, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockReadINTEL, true, true, IR(172, 4), IR(42391, 28), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageBlockWriteINTEL, false, false, IR(33, 3), IR(42419, 29), IR(0, 0), IR(303, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockReadINTEL, true, true, IR(215, 6), IR(42448, 33), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupImageMediaBlockWriteINTEL, false, false, IR(675, 5), IR(42481, 34), IR(0, 0), IR(304, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpUCountLeadingZerosINTEL, true, true, IR(160, 3), IR(42515, 24), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUCountTrailingZerosINTEL, true, true, IR(160, 3), IR(42539, 25), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsISubINTEL, true, true, IR(172, 4), IR(42564, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAbsUSubINTEL, true, true, IR(172, 4), IR(42577, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAddSatINTEL, true, true, IR(172, 4), IR(42590, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAddSatINTEL, true, true, IR(172, 4), IR(42603, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageINTEL, true, true, IR(172, 4), IR(42616, 14), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageINTEL, true, true, IR(172, 4), IR(42630, 14), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIAverageRoundedINTEL, true, true, IR(172, 4), IR(42644, 21), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUAverageRoundedINTEL, true, true, IR(172, 4), IR(42665, 21), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpISubSatINTEL, true, true, IR(172, 4), IR(42686, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUSubSatINTEL, true, true, IR(172, 4), IR(42699, 13), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpIMul32x16INTEL, true, true, IR(172, 4), IR(42712, 15), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpUMul32x16INTEL, true, true, IR(172, 4), IR(42727, 15), IR(0, 0), IR(305, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpConstantFunctionPointerINTEL, true, true, IR(160, 3), IR(42742, 29), IR(0, 0), IR(218, 1), IR(115, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFunctionPointerCallINTEL, true, true, IR(107, 3), IR(42771, 25), IR(0, 0), IR(218, 1), IR(115, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmTargetINTEL, true, false, IR(50, 2), IR(42796, 15), IR(0, 0), IR(140, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmINTEL, true, true, IR(680, 6), IR(14179, 9), IR(0, 0), IR(140, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAsmCallINTEL, true, true, IR(122, 4), IR(42811, 13), IR(0, 0), IR(140, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAtomicFMinEXT, true, true, IR(235, 6), IR(42824, 14), IR(0, 0), IR(306, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAtomicFMaxEXT, true, true, IR(235, 6), IR(42838, 14), IR(0, 0), IR(306, 4), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpAssumeTrueKHR, false, false, IR(10, 1), IR(42852, 14), IR(0, 0), IR(310, 1), IR(119, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpExpectKHR, true, true, IR(172, 4), IR(42866, 10), IR(0, 0), IR(310, 1), IR(119, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpDecorateString, false, false, IR(163, 2), IR(42876, 15), IR(190, 1), IR(1, 0), IR(223, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpMemberDecorateString, false, false, IR(165, 3), IR(42912, 21), IR(191, 1), IR(1, 0), IR(223, 2), SPV_SPIRV_VERSION_WORD(1,4), 0xffffffffu, PrintingClass::kAnnotation},
  {spv::Op::OpVmeImageINTEL, true, true, IR(172, 4), IR(42960, 14), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeVmeImageINTEL, true, false, IR(87, 2), IR(42974, 18), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImePayloadINTEL, true, false, IR(68, 1), IR(42992, 23), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefPayloadINTEL, true, false, IR(68, 1), IR(43015, 23), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicPayloadINTEL, true, false, IR(68, 1), IR(43038, 23), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMcePayloadINTEL, true, false, IR(68, 1), IR(43061, 23), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcMceResultINTEL, true, false, IR(68, 1), IR(43084, 22), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultINTEL, true, false, IR(68, 1), IR(43106, 22), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultSingleReferenceStreamoutINTEL, true, false, IR(68, 1), IR(43128, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeResultDualReferenceStreamoutINTEL, true, false, IR(68, 1), IR(43174, 44), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeSingleReferenceStreaminINTEL, true, false, IR(68, 1), IR(43218, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcImeDualReferenceStreaminINTEL, true, false, IR(68, 1), IR(43257, 37), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcRefResultINTEL, true, false, IR(68, 1), IR(43294, 22), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeAvcSicResultINTEL, true, false, IR(68, 1), IR(43316, 22), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(43338, 60), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL, true, true, IR(172, 4), IR(43398, 53), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterShapePenaltyINTEL, true, true, IR(172, 4), IR(43451, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterShapePenaltyINTEL, true, true, IR(172, 4), IR(43498, 40), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(43538, 51), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetInterDirectionPenaltyINTEL, true, true, IR(172, 4), IR(43589, 44), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(43633, 51), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL, true, true, IR(172, 4), IR(43684, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43740, 50), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43790, 52), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL, true, true, IR(39, 2), IR(43842, 49), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetMotionVectorCostFunctionINTEL, true, true, IR(215, 6), IR(43891, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL, true, true, IR(172, 4), IR(43938, 50), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL, true, true, IR(39, 2), IR(43988, 51), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL, true, true, IR(39, 2), IR(44039, 56), IR(0, 0), IR(314, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetAcOnlyHaarINTEL, true, true, IR(160, 3), IR(44095, 33), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(44128, 52), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL, true, true, IR(172, 4), IR(44180, 61), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(44241, 61), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImePayloadINTEL, true, true, IR(160, 3), IR(44302, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToImeResultINTEL, true, true, IR(160, 3), IR(44341, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefPayloadINTEL, true, true, IR(160, 3), IR(44379, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToRefResultINTEL, true, true, IR(160, 3), IR(44418, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicPayloadINTEL, true, true, IR(160, 3), IR(44456, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceConvertToSicResultINTEL, true, true, IR(160, 3), IR(44495, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetMotionVectorsINTEL, true, true, IR(160, 3), IR(44533, 36), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDistortionsINTEL, true, true, IR(160, 3), IR(44569, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetBestInterDistortionsINTEL, true, true, IR(160, 3), IR(44608, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMajorShapeINTEL, true, true, IR(160, 3), IR(44651, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMinorShapeINTEL, true, true, IR(160, 3), IR(44689, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterDirectionsINTEL, true, true, IR(160, 3), IR(44727, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterMotionVectorCountINTEL, true, true, IR(160, 3), IR(44765, 45), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceIdsINTEL, true, true, IR(160, 3), IR(44810, 40), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL, true, true, IR(130, 5), IR(44850, 62), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeInitializeINTEL, true, true, IR(130, 5), IR(44912, 30), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetSingleReferenceINTEL, true, true, IR(130, 5), IR(44942, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetDualReferenceINTEL, true, true, IR(215, 6), IR(44980, 36), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeRefWindowSizeINTEL, true, true, IR(172, 4), IR(45016, 33), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeAdjustRefOffsetINTEL, true, true, IR(215, 6), IR(45049, 35), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(45084, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetMaxMotionVectorCountINTEL, true, true, IR(172, 4), IR(45123, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetUnidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(45166, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetEarlySearchTerminationThresholdINTEL, true, true, IR(172, 4), IR(45213, 54), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeSetWeightedSadINTEL, true, true, IR(172, 4), IR(45267, 34), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(45301, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(45348, 45), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL, true, true, IR(215, 6), IR(45393, 55), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL, true, true, IR(325, 7), IR(45448, 53), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL, true, true, IR(130, 5), IR(45501, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL, true, true, IR(215, 6), IR(45557, 54), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL, true, true, IR(215, 6), IR(45611, 58), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL, true, true, IR(325, 7), IR(45669, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeConvertToMceResultINTEL, true, true, IR(160, 3), IR(45725, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetSingleReferenceStreaminINTEL, true, true, IR(160, 3), IR(45763, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetDualReferenceStreaminINTEL, true, true, IR(160, 3), IR(45809, 44), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripSingleReferenceStreamoutINTEL, true, true, IR(160, 3), IR(45853, 49), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeStripDualReferenceStreamoutINTEL, true, true, IR(160, 3), IR(45902, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL, true, true, IR(172, 4), IR(45949, 70), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL, true, true, IR(172, 4), IR(46019, 68), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL, true, true, IR(172, 4), IR(46087, 69), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL, true, true, IR(130, 5), IR(46156, 68), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL, true, true, IR(130, 5), IR(46224, 66), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL, true, true, IR(130, 5), IR(46290, 67), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetBorderReachedINTEL, true, true, IR(172, 4), IR(46357, 36), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetTruncatedSearchIndicationINTEL, true, true, IR(160, 3), IR(46393, 48), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL, true, true, IR(160, 3), IR(46441, 59), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL, true, true, IR(160, 3), IR(46500, 58), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL, true, true, IR(160, 3), IR(46558, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcFmeInitializeINTEL, true, true, IR(686, 9), IR(46614, 30), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcBmeInitializeINTEL, true, true, IR(695, 10), IR(46644, 30), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(46674, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBidirectionalMixDisableINTEL, true, true, IR(160, 3), IR(46713, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(46759, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(46802, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(46849, 45), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(46894, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(46940, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcRefConvertToMceResultINTEL, true, true, IR(160, 3), IR(46996, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicInitializeINTEL, true, true, IR(160, 3), IR(47034, 30), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureSkcINTEL, true, true, IR(288, 8), IR(47064, 32), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaINTEL, true, true, IR(695, 10), IR(47096, 36), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConfigureIpeLumaChromaINTEL, true, true, IR(705, 13), IR(47132, 42), IR(0, 0), IR(314, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetMotionVectorMaskINTEL, true, true, IR(172, 4), IR(47174, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMcePayloadINTEL, true, true, IR(160, 3), IR(47213, 39), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaShapePenaltyINTEL, true, true, IR(172, 4), IR(47252, 44), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraLumaModeCostFunctionINTEL, true, true, IR(215, 6), IR(47296, 48), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetIntraChromaModeCostFunctionINTEL, true, true, IR(172, 4), IR(47344, 50), IR(0, 0), IR(314, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBilinearFilterEnableINTEL, true, true, IR(160, 3), IR(47394, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetSkcForwardTransformEnableINTEL, true, true, IR(172, 4), IR(47437, 48), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicSetBlockBasedRawSkipSadINTEL, true, true, IR(172, 4), IR(47485, 43), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateIpeINTEL, true, true, IR(172, 4), IR(47528, 31), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithSingleReferenceINTEL, true, true, IR(130, 5), IR(47559, 47), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithDualReferenceINTEL, true, true, IR(215, 6), IR(47606, 45), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceINTEL, true, true, IR(130, 5), IR(47651, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL, true, true, IR(215, 6), IR(47697, 56), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicConvertToMceResultINTEL, true, true, IR(160, 3), IR(47753, 38), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeLumaShapeINTEL, true, true, IR(160, 3), IR(47791, 35), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeLumaDistortionINTEL, true, true, IR(160, 3), IR(47826, 44), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetBestIpeChromaDistortionINTEL, true, true, IR(160, 3), IR(47870, 46), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedIpeLumaModesINTEL, true, true, IR(160, 3), IR(47916, 41), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetIpeChromaModeINTEL, true, true, IR(160, 3), IR(47957, 36), IR(0, 0), IR(314, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL, true, true, IR(160, 3), IR(47993, 50), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL, true, true, IR(160, 3), IR(48043, 48), IR(0, 0), IR(312, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupAvcSicGetInterRawSadsINTEL, true, true, IR(160, 3), IR(48091, 35), IR(0, 0), IR(311, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpVariableLengthArrayINTEL, true, true, IR(160, 3), IR(14417, 25), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpSaveMemoryINTEL, true, true, IR(39, 2), IR(48126, 16), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpRestoreMemoryINTEL, false, false, IR(10, 1), IR(48142, 19), IR(0, 0), IR(316, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpArbitraryFloatSinCosPiALTERA, true, true, IR(718, 8), IR(48161, 29), IR(192, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastALTERA, true, true, IR(718, 8), IR(48218, 25), IR(193, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastFromIntALTERA, true, true, IR(718, 8), IR(48267, 32), IR(194, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCastToIntALTERA, true, true, IR(718, 8), IR(48330, 30), IR(195, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatAddALTERA, true, true, IR(726, 10), IR(48389, 24), IR(196, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSubALTERA, true, true, IR(726, 10), IR(48436, 24), IR(197, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatMulALTERA, true, true, IR(726, 10), IR(48483, 24), IR(198, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatDivALTERA, true, true, IR(726, 10), IR(48530, 24), IR(199, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGTALTERA, true, true, IR(736, 6), IR(48577, 23), IR(200, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatGEALTERA, true, true, IR(736, 6), IR(48622, 23), IR(201, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLTALTERA, true, true, IR(736, 6), IR(48667, 23), IR(202, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLEALTERA, true, true, IR(736, 6), IR(48712, 23), IR(203, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatEQALTERA, true, true, IR(736, 6), IR(48757, 23), IR(204, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRecipALTERA, true, true, IR(718, 8), IR(48802, 26), IR(205, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatRSqrtALTERA, true, true, IR(718, 8), IR(48853, 26), IR(206, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCbrtALTERA, true, true, IR(718, 8), IR(48904, 25), IR(207, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatHypotALTERA, true, true, IR(726, 10), IR(48953, 26), IR(208, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSqrtALTERA, true, true, IR(718, 8), IR(49004, 25), IR(209, 1), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLogINTEL, true, true, IR(718, 8), IR(49053, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog2INTEL, true, true, IR(718, 8), IR(49076, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog10INTEL, true, true, IR(718, 8), IR(49100, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatLog1pINTEL, true, true, IR(718, 8), IR(49125, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpINTEL, true, true, IR(718, 8), IR(49150, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp2INTEL, true, true, IR(718, 8), IR(49173, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExp10INTEL, true, true, IR(718, 8), IR(49197, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatExpm1INTEL, true, true, IR(718, 8), IR(49222, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinINTEL, true, true, IR(718, 8), IR(49247, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosINTEL, true, true, IR(718, 8), IR(49270, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinCosINTEL, true, true, IR(718, 8), IR(49293, 26), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatSinPiINTEL, true, true, IR(718, 8), IR(49319, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatCosPiINTEL, true, true, IR(718, 8), IR(49344, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinINTEL, true, true, IR(718, 8), IR(49369, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatASinPiINTEL, true, true, IR(718, 8), IR(49393, 26), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosINTEL, true, true, IR(718, 8), IR(49419, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatACosPiINTEL, true, true, IR(718, 8), IR(49443, 26), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanINTEL, true, true, IR(718, 8), IR(49469, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATanPiINTEL, true, true, IR(718, 8), IR(49493, 26), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatATan2INTEL, true, true, IR(726, 10), IR(49519, 25), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowINTEL, true, true, IR(726, 10), IR(49544, 23), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowRINTEL, true, true, IR(726, 10), IR(49567, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpArbitraryFloatPowNINTEL, true, true, IR(726, 10), IR(49591, 24), IR(0, 0), IR(317, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpLoopControlINTEL, false, false, IR(18, 1), IR(49615, 17), IR(0, 0), IR(318, 1), IR(129, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAliasDomainDeclINTEL, true, false, IR(742, 2), IR(49632, 21), IR(0, 0), IR(146, 1), IR(141, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeDeclINTEL, true, false, IR(744, 3), IR(49653, 20), IR(0, 0), IR(146, 1), IR(141, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpAliasScopeListDeclINTEL, true, false, IR(92, 2), IR(49673, 24), IR(0, 0), IR(146, 1), IR(141, 1), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSqrtALTERA, true, true, IR(718, 8), IR(49697, 16), IR(210, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRecipALTERA, true, true, IR(718, 8), IR(49728, 17), IR(211, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedRsqrtALTERA, true, true, IR(718, 8), IR(49761, 17), IR(212, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinALTERA, true, true, IR(718, 8), IR(49794, 15), IR(213, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosALTERA, true, true, IR(718, 8), IR(49823, 15), IR(214, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosALTERA, true, true, IR(718, 8), IR(49852, 18), IR(215, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinPiALTERA, true, true, IR(718, 8), IR(49887, 17), IR(216, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedCosPiALTERA, true, true, IR(718, 8), IR(49920, 17), IR(217, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedSinCosPiALTERA, true, true, IR(718, 8), IR(49953, 20), IR(218, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedLogALTERA, true, true, IR(718, 8), IR(49992, 15), IR(219, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFixedExpALTERA, true, true, IR(718, 8), IR(50021, 15), IR(220, 1), IR(208, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpPtrCastToCrossWorkgroupALTERA, true, true, IR(160, 3), IR(50050, 30), IR(221, 1), IR(219, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpCrossWorkgroupCastToPtrALTERA, true, true, IR(160, 3), IR(50109, 30), IR(222, 1), IR(219, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpReadPipeBlockingALTERA, false, false, IR(296, 4), IR(50168, 23), IR(223, 1), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpWritePipeBlockingALTERA, false, false, IR(296, 4), IR(50213, 24), IR(224, 1), IR(319, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpFPGARegALTERA, true, true, IR(160, 3), IR(15375, 14), IR(65, 1), IR(320, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpRayQueryGetRayTMinKHR, true, true, IR(160, 3), IR(50260, 22), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetRayFlagsKHR, true, true, IR(160, 3), IR(50282, 23), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionTKHR, true, true, IR(172, 4), IR(50305, 28), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceCustomIndexKHR, true, true, IR(172, 4), IR(50333, 46), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceIdKHR, true, true, IR(172, 4), IR(50379, 37), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR, true, true, IR(172, 4), IR(50416, 65), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionGeometryIndexKHR, true, true, IR(172, 4), IR(50481, 40), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionPrimitiveIndexKHR, true, true, IR(172, 4), IR(50521, 41), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionBarycentricsKHR, true, true, IR(172, 4), IR(50562, 39), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionFrontFaceKHR, true, true, IR(172, 4), IR(50601, 36), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionCandidateAABBOpaqueKHR, true, true, IR(160, 3), IR(50637, 46), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayDirectionKHR, true, true, IR(172, 4), IR(50683, 45), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectRayOriginKHR, true, true, IR(172, 4), IR(50728, 42), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayDirectionKHR, true, true, IR(160, 3), IR(50770, 32), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetWorldRayOriginKHR, true, true, IR(160, 3), IR(50802, 29), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionObjectToWorldKHR, true, true, IR(172, 4), IR(50831, 40), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpRayQueryGetIntersectionWorldToObjectKHR, true, true, IR(172, 4), IR(50871, 40), IR(0, 0), IR(212, 1), IR(49, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpAtomicFAddEXT, true, true, IR(235, 6), IR(50911, 14), IR(0, 0), IR(321, 4), IR(166, 1), 0xffffffffu, 0xffffffffu, PrintingClass::kAtomic},
  {spv::Op::OpTypeBufferSurfaceINTEL, true, false, IR(100, 2), IR(50925, 23), IR(0, 0), IR(138, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpTypeStructContinuedINTEL, false, false, IR(747, 1), IR(50948, 25), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kType_Declaration},
  {spv::Op::OpConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(50973, 32), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCompositeContinuedINTEL, false, false, IR(747, 1), IR(51005, 36), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpCompositeConstructContinuedINTEL, false, false, IR(747, 1), IR(51041, 33), IR(0, 0), IR(325, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpConvertFToBF16INTEL, true, true, IR(160, 3), IR(51074, 20), IR(0, 0), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpConvertBF16ToFINTEL, true, true, IR(160, 3), IR(51094, 20), IR(0, 0), IR(326, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpControlBarrierArriveEXT, false, false, IR(221, 3), IR(51114, 24), IR(225, 1), IR(327, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpControlBarrierWaitEXT, false, false, IR(221, 3), IR(51164, 22), IR(226, 1), IR(327, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBarrier},
  {spv::Op::OpArithmeticFenceEXT, true, true, IR(160, 3), IR(15886, 19), IR(0, 0), IR(328, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMiscellaneous},
  {spv::Op::OpTaskSequenceCreateALTERA, true, true, IR(748, 7), IR(51210, 25), IR(227, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceAsyncALTERA, false, false, IR(168, 2), IR(51259, 24), IR(228, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceGetALTERA, true, true, IR(160, 3), IR(51306, 22), IR(229, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTaskSequenceReleaseALTERA, false, false, IR(10, 1), IR(51349, 26), IR(230, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpTypeTaskSequenceALTERA, true, false, IR(68, 1), IR(51400, 23), IR(231, 1), IR(329, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::k_exclude},
  {spv::Op::OpSubgroupBlockPrefetchINTEL, false, false, IR(139, 3), IR(51445, 27), IR(0, 0), IR(330, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadINTEL, false, false, IR(755, 10), IR(51472, 25), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransformINTEL, false, false, IR(755, 10), IR(51497, 34), IR(0, 0), IR(331, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockLoadTransposeINTEL, false, false, IR(755, 10), IR(51531, 34), IR(0, 0), IR(332, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockPrefetchINTEL, false, false, IR(765, 9), IR(51565, 29), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroup2DBlockStoreINTEL, false, false, IR(755, 10), IR(51594, 26), IR(0, 0), IR(113, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpSubgroupMatrixMultiplyAccumulateINTEL, true, true, IR(774, 7), IR(16363, 38), IR(0, 0), IR(333, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpBitwiseFunctionINTEL, true, true, IR(215, 6), IR(51682, 21), IR(0, 0), IR(334, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kBit},
  {spv::Op::OpUntypedVariableLengthArrayINTEL, true, true, IR(172, 4), IR(16429, 32), IR(0, 0), IR(335, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConditionalExtensionINTEL, false, false, IR(45, 2), IR(51703, 26), IR(0, 0), IR(116, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kExtension},
  {spv::Op::OpConditionalEntryPointINTEL, false, false, IR(781, 5), IR(51729, 27), IR(0, 0), IR(116, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpConditionalCapabilityINTEL, false, false, IR(786, 2), IR(51756, 27), IR(0, 0), IR(116, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMode_Setting},
  {spv::Op::OpSpecConstantTargetINTEL, true, true, IR(788, 4), IR(51783, 24), IR(0, 0), IR(336, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantArchitectureINTEL, true, true, IR(792, 6), IR(51807, 30), IR(0, 0), IR(336, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpSpecConstantCapabilitiesINTEL, true, true, IR(798, 3), IR(51874, 30), IR(0, 0), IR(336, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConstant_Creation},
  {spv::Op::OpConditionalCopyObjectINTEL, true, true, IR(107, 3), IR(51904, 27), IR(0, 0), IR(116, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kComposite},
  {spv::Op::OpPredicatedLoadINTEL, true, true, IR(636, 6), IR(51931, 20), IR(0, 0), IR(337, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpPredicatedStoreINTEL, false, false, IR(589, 4), IR(51951, 21), IR(0, 0), IR(337, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpGroupIMulKHR, true, true, IR(283, 5), IR(51972, 13), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupFMulKHR, true, true, IR(283, 5), IR(51985, 13), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseAndKHR, true, true, IR(283, 5), IR(51998, 19), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseOrKHR, true, true, IR(283, 5), IR(52017, 18), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupBitwiseXorKHR, true, true, IR(283, 5), IR(52035, 19), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalAndKHR, true, true, IR(283, 5), IR(52054, 19), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalOrKHR, true, true, IR(283, 5), IR(52073, 18), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpGroupLogicalXorKHR, true, true, IR(283, 5), IR(52091, 19), IR(0, 0), IR(338, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kGroup},
  {spv::Op::OpRoundFToTF32INTEL, true, true, IR(160, 3), IR(52110, 18), IR(0, 0), IR(339, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kConversion},
  {spv::Op::OpMaskedGatherINTEL, true, true, IR(801, 6), IR(52128, 18), IR(0, 0), IR(340, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpMaskedScatterINTEL, false, false, IR(807, 4), IR(52146, 19), IR(0, 0), IR(340, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kMemory},
  {spv::Op::OpConvertHandleToImageINTEL, true, true, IR(160, 3), IR(52165, 26), IR(0, 0), IR(341, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSamplerINTEL, true, true, IR(160, 3), IR(52191, 28), IR(0, 0), IR(341, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpConvertHandleToSampledImageINTEL, true, true, IR(160, 3), IR(52219, 33), IR(0, 0), IR(341, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kImage},
  {spv::Op::OpFDot2MixAcc32VALVE, true, true, IR(130, 5), IR(52252, 19), IR(0, 0), IR(342, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot2MixAcc16VALVE, true, true, IR(130, 5), IR(52271, 19), IR(0, 0), IR(344, 2), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
  {spv::Op::OpFDot4MixAcc32VALVE, true, true, IR(130, 5), IR(52290, 19), IR(0, 0), IR(346, 1), IR(0, 0), 0xffffffffu, 0xffffffffu, PrintingClass::kReserved},
}};

// Opcode strings (without the 'Op' prefix) and opcode values, ordered by name.
// The fields in order are:
//   name, either the primary name or an alias, indexing into kStrings
//   index into kInstructionDesc
static const std::array<NameIndex, 935> kInstructionNames{{
  {IR(11690, 9), 441}, // 0 AbortKHR
  {IR(42564, 13), 604}, // 1 AbsISubINTEL
  {IR(42577, 13), 605}, // 2 AbsUSubINTEL
  {IR(31046, 12), 58}, // 3 AccessChain
  {IR(49632, 21), 790}, // 4 AliasDomainDeclINTEL
  {IR(49653, 20), 791}, // 5 AliasScopeDeclINTEL
  {IR(49673, 24), 792}, // 6 AliasScopeListDeclINTEL
  {IR(32343, 4), 143}, // 7 All
  {IR(37738, 25), 429}, // 8 AllocateNodePayloadsAMDX
  {IR(32339, 4), 142}, // 9 Any
  {IR(49419, 24), 781}, // 10 ArbitraryFloatACosINTEL
  {IR(49443, 26), 782}, // 11 ArbitraryFloatACosPiINTEL
  {IR(49369, 24), 779}, // 12 ArbitraryFloatASinINTEL
  {IR(49393, 26), 780}, // 13 ArbitraryFloatASinPiINTEL
  {IR(49519, 25), 785}, // 14 ArbitraryFloatATan2INTEL
  {IR(49469, 24), 783}, // 15 ArbitraryFloatATanINTEL
  {IR(49493, 26), 784}, // 16 ArbitraryFloatATanPiINTEL
  {IR(48389, 24), 752}, // 17 ArbitraryFloatAddALTERA
  {IR(48413, 23), 752}, // 18 ArbitraryFloatAddINTEL
  {IR(48218, 25), 749}, // 19 ArbitraryFloatCastALTERA
  {IR(48267, 32), 750}, // 20 ArbitraryFloatCastFromIntALTERA
  {IR(48299, 31), 750}, // 21 ArbitraryFloatCastFromIntINTEL
  {IR(48243, 24), 749}, // 22 ArbitraryFloatCastINTEL
  {IR(48330, 30), 751}, // 23 ArbitraryFloatCastToIntALTERA
  {IR(48360, 29), 751}, // 24 ArbitraryFloatCastToIntINTEL
  {IR(48904, 25), 763}, // 25 ArbitraryFloatCbrtALTERA
  {IR(48929, 24), 763}, // 26 ArbitraryFloatCbrtINTEL
  {IR(49270, 23), 775}, // 27 ArbitraryFloatCosINTEL
  {IR(49344, 25), 778}, // 28 ArbitraryFloatCosPiINTEL
  {IR(48530, 24), 755}, // 29 ArbitraryFloatDivALTERA
  {IR(48554, 23), 755}, // 30 ArbitraryFloatDivINTEL
  {IR(48757, 23), 760}, // 31 ArbitraryFloatEQALTERA
  {IR(48780, 22), 760}, // 32 ArbitraryFloatEQINTEL
  {IR(49197, 25), 772}, // 33 ArbitraryFloatExp10INTEL
  {IR(49173, 24), 771}, // 34 ArbitraryFloatExp2INTEL
  {IR(49150, 23), 770}, // 35 ArbitraryFloatExpINTEL
  {IR(49222, 25), 773}, // 36 ArbitraryFloatExpm1INTEL
  {IR(48622, 23), 757}, // 37 ArbitraryFloatGEALTERA
  {IR(48645, 22), 757}, // 38 ArbitraryFloatGEINTEL
  {IR(48577, 23), 756}, // 39 ArbitraryFloatGTALTERA
  {IR(48600, 22), 756}, // 40 ArbitraryFloatGTINTEL
  {IR(48953, 26), 764}, // 41 ArbitraryFloatHypotALTERA
  {IR(48979, 25), 764}, // 42 ArbitraryFloatHypotINTEL
  {IR(48712, 23), 759}, // 43 ArbitraryFloatLEALTERA
  {IR(48735, 22), 759}, // 44 ArbitraryFloatLEINTEL
  {IR(48667, 23), 758}, // 45 ArbitraryFloatLTALTERA
  {IR(48690, 22), 758}, // 46 ArbitraryFloatLTINTEL
  {IR(49100, 25), 768}, // 47 ArbitraryFloatLog10INTEL
  {IR(49125, 25), 769}, // 48 ArbitraryFloatLog1pINTEL
  {IR(49076, 24), 767}, // 49 ArbitraryFloatLog2INTEL
  {IR(49053, 23), 766}, // 50 ArbitraryFloatLogINTEL
  {IR(48483, 24), 754}, // 51 ArbitraryFloatMulALTERA
  {IR(48507, 23), 754}, // 52 ArbitraryFloatMulINTEL
  {IR(49544, 23), 786}, // 53 ArbitraryFloatPowINTEL
  {IR(49591, 24), 788}, // 54 ArbitraryFloatPowNINTEL
  {IR(49567, 24), 787}, // 55 ArbitraryFloatPowRINTEL
  {IR(48853, 26), 762}, // 56 ArbitraryFloatRSqrtALTERA
  {IR(48879, 25), 762}, // 57 ArbitraryFloatRSqrtINTEL
  {IR(48802, 26), 761}, // 58 ArbitraryFloatRecipALTERA
  {IR(48828, 25), 761}, // 59 ArbitraryFloatRecipINTEL
  {IR(49293, 26), 776}, // 60 ArbitraryFloatSinCosINTEL
  {IR(48161, 29), 748}, // 61 ArbitraryFloatSinCosPiALTERA
  {IR(48190, 28), 748}, // 62 ArbitraryFloatSinCosPiINTEL
  {IR(49247, 23), 774}, // 63 ArbitraryFloatSinINTEL
  {IR(49319, 25), 777}, // 64 ArbitraryFloatSinPiINTEL
  {IR(49004, 25), 765}, // 65 ArbitraryFloatSqrtALTERA
  {IR(49029, 24), 765}, // 66 ArbitraryFloatSqrtINTEL
  {IR(48436, 24), 753}, // 67 ArbitraryFloatSubALTERA
  {IR(48460, 23), 753}, // 68 ArbitraryFloatSubINTEL
  {IR(15886, 19), 836}, // 69 ArithmeticFenceEXT
  {IR(31093, 12), 61}, // 70 ArrayLength
  {IR(42811, 13), 620}, // 71 AsmCallINTEL
  {IR(14179, 9), 619}, // 72 AsmINTEL
  {IR(42796, 15), 618}, // 73 AsmTargetINTEL
  {IR(42852, 14), 623}, // 74 AssumeTrueKHR
  {IR(33365, 10), 220}, // 75 AtomicAnd
  {IR(33217, 22), 210}, // 76 AtomicCompareExchange
  {IR(33239, 26), 211}, // 77 AtomicCompareExchangeWeak
  {IR(33202, 15), 209}, // 78 AtomicExchange
  {IR(50911, 14), 826}, // 79 AtomicFAddEXT
  {IR(42838, 14), 622}, // 80 AtomicFMaxEXT
  {IR(42824, 14), 621}, // 81 AtomicFMinEXT
  {IR(34750, 16), 292}, // 82 AtomicFlagClear
  {IR(34729, 21), 291}, // 83 AtomicFlagTestAndSet
  {IR(33299, 11), 214}, // 84 AtomicIAdd
  {IR(33282, 17), 213}, // 85 AtomicIDecrement
  {IR(33265, 17), 212}, // 86 AtomicIIncrement
  {IR(33310, 11), 215}, // 87 AtomicISub
  {IR(33179, 11), 207}, // 88 AtomicLoad
  {IR(33375, 9), 221}, // 89 AtomicOr
  {IR(33343, 11), 218}, // 90 AtomicSMax
  {IR(33321, 11), 216}, // 91 AtomicSMin
  {IR(33190, 12), 208}, // 92 AtomicStore
  {IR(33354, 11), 219}, // 93 AtomicUMax
  {IR(33332, 11), 217}, // 94 AtomicUMin
  {IR(33384, 10), 222}, // 95 AtomicXor
  {IR(11353, 19), 413}, // 96 BFloat16MulAddQCOM
  {IR(41000, 28), 550}, // 97 BeginInvocationInterlockEXT
  {IR(37276, 17), 408}, // 98 BitCastArrayQCOM
  {IR(32963, 9), 191}, // 99 BitCount
  {IR(32903, 15), 187}, // 100 BitFieldInsert
  {IR(32918, 17), 188}, // 101 BitFieldSExtract
  {IR(32935, 17), 189}, // 102 BitFieldUExtract
  {IR(32952, 11), 190}, // 103 BitReverse
  {IR(32091, 8), 114}, // 104 Bitcast
  {IR(10580, 18), 358}, // 105 BitcastExtractEXT
  {IR(32888, 11), 185}, // 106 BitwiseAnd
  {IR(51682, 21), 849}, // 107 BitwiseFunctionINTEL
  {IR(32867, 10), 183}, // 108 BitwiseOr
  {IR(32877, 11), 184}, // 109 BitwiseXor
  {IR(33494, 7), 227}, // 110 Branch
  {IR(33501, 18), 228}, // 111 BranchConditional
  {IR(37999, 17), 440}, // 112 BufferPointerEXT
  {IR(34362, 13), 277}, // 113 BuildNDRange
  {IR(30107, 11), 15}, // 114 Capability
  {IR(34320, 26), 275}, // 115 CaptureEventProfilingInfo
  {IR(35826, 23), 344}, // 116 ColorAttachmentReadEXT
  {IR(33912, 15), 255}, // 117 CommitReadPipe
  {IR(33927, 16), 256}, // 118 CommitWritePipe
  {IR(31353, 19), 72}, // 119 CompositeConstruct
  {IR(51041, 33), 831}, // 120 CompositeConstructContinuedINTEL
  {IR(37409, 30), 414}, // 121 CompositeConstructCoopMatQCOM
  {IR(36984, 31), 396}, // 122 CompositeConstructReplicateEXT
  {IR(31372, 17), 73}, // 123 CompositeExtract
  {IR(37439, 28), 415}, // 124 CompositeExtractCoopMatQCOM
  {IR(31389, 16), 74}, // 125 CompositeInsert
  {IR(51756, 27), 853}, // 126 ConditionalCapabilityINTEL
  {IR(51904, 27), 857}, // 127 ConditionalCopyObjectINTEL
  {IR(51729, 27), 852}, // 128 ConditionalEntryPointINTEL
  {IR(51703, 26), 851}, // 129 ConditionalExtensionINTEL
  {IR(18202, 9), 39}, // 130 Constant
  {IR(30613, 18), 40}, // 131 ConstantComposite
  {IR(50973, 32), 829}, // 132 ConstantCompositeContinuedINTEL
  {IR(36920, 30), 394}, // 133 ConstantCompositeReplicateEXT
  {IR(11699, 16), 445}, // 134 ConstantDataKHR
  {IR(30561, 14), 38}, // 135 ConstantFalse
  {IR(42742, 29), 616}, // 136 ConstantFunctionPointerINTEL
  {IR(30725, 13), 42}, // 137 ConstantNull
  {IR(34805, 20), 296}, // 138 ConstantPipeStorage
  {IR(30709, 16), 41}, // 139 ConstantSampler
  {IR(38064, 18), 444}, // 140 ConstantSizeOfEXT
  {IR(37891, 19), 435}, // 141 ConstantStringAMDX
  {IR(30548, 13), 37}, // 142 ConstantTrue
  {IR(33150, 15), 205}, // 143 ControlBarrier
  {IR(51114, 24), 834}, // 144 ControlBarrierArriveEXT
  {IR(51138, 26), 834}, // 145 ControlBarrierArriveINTEL
  {IR(51164, 22), 835}, // 146 ControlBarrierWaitEXT
  {IR(51186, 24), 835}, // 147 ControlBarrierWaitINTEL
  {IR(51094, 20), 833}, // 148 ConvertBF16ToFINTEL
  {IR(51074, 20), 832}, // 149 ConvertFToBF16INTEL
  {IR(31897, 12), 100}, // 150 ConvertFToS
  {IR(31885, 12), 99}, // 151 ConvertFToU
  {IR(52165, 26), 871}, // 152 ConvertHandleToImageINTEL
  {IR(52219, 33), 873}, // 153 ConvertHandleToSampledImageINTEL
  {IR(52191, 28), 872}, // 154 ConvertHandleToSamplerINTEL
  {IR(41709, 18), 573}, // 155 ConvertImageToUNV
  {IR(31974, 14), 107}, // 156 ConvertPtrToU
  {IR(31909, 12), 101}, // 157 ConvertSToF
  {IR(41772, 25), 576}, // 158 ConvertSampledImageToUNV
  {IR(41727, 20), 574}, // 159 ConvertSamplerToUNV
  {IR(36498, 35), 380}, // 160 ConvertUToAccelerationStructureKHR
  {IR(31921, 12), 102}, // 161 ConvertUToF
  {IR(41671, 18), 571}, // 162 ConvertUToImageNV
  {IR(32018, 14), 110}, // 163 ConvertUToPtr
  {IR(41747, 25), 575}, // 164 ConvertUToSampledImageNV
  {IR(41689, 20), 572}, // 165 ConvertUToSamplerNV
  {IR(39235, 27), 488}, // 166 CooperativeMatrixConvertNV
  {IR(39204, 31), 488}, // 167 CooperativeMatrixConvertUseEXT
  {IR(13876, 34), 549}, // 168 CooperativeMatrixGetCoordinateEXT
  {IR(36893, 27), 393}, // 169 CooperativeMatrixLengthKHR
  {IR(40974, 26), 548}, // 170 CooperativeMatrixLengthNV
  {IR(36761, 25), 390}, // 171 CooperativeMatrixLoadKHR
  {IR(40899, 24), 545}, // 172 CooperativeMatrixLoadNV
  {IR(41225, 30), 553}, // 173 CooperativeMatrixLoadTensorNV
  {IR(36866, 27), 392}, // 174 CooperativeMatrixMulAddKHR
  {IR(40948, 26), 547}, // 175 CooperativeMatrixMulAddNV
  {IR(41286, 33), 555}, // 176 CooperativeMatrixPerElementOpEXT
  {IR(41319, 32), 555}, // 177 CooperativeMatrixPerElementOpNV
  {IR(41097, 27), 552}, // 178 CooperativeMatrixReduceEXT
  {IR(41124, 26), 552}, // 179 CooperativeMatrixReduceNV
  {IR(36786, 26), 391}, // 180 CooperativeMatrixStoreKHR
  {IR(40923, 25), 546}, // 181 CooperativeMatrixStoreNV
  {IR(41255, 31), 554}, // 182 CooperativeMatrixStoreTensorNV
  {IR(41642, 29), 570}, // 183 CooperativeMatrixTransposeNV
  {IR(39458, 24), 495}, // 184 CooperativeVectorLoadNV
  {IR(39172, 32), 487}, // 185 CooperativeVectorMatrixMulAddNV
  {IR(39062, 29), 484}, // 186 CooperativeVectorMatrixMulNV
  {IR(39091, 42), 485}, // 187 CooperativeVectorOuterProductAccumulateNV
  {IR(39133, 39), 486}, // 188 CooperativeVectorReduceSumAccumulateNV
  {IR(39482, 25), 496}, // 189 CooperativeVectorStoreNV
  {IR(35785, 12), 340}, // 190 CopyLogical
  {IR(31019, 11), 56}, // 191 CopyMemory
  {IR(31030, 16), 57}, // 192 CopyMemorySized
  {IR(31405, 11), 75}, // 193 CopyObject
  {IR(34825, 26), 297}, // 194 CreatePipeFromPipeStorage
  {IR(41387, 21), 558}, // 195 CreateTensorLayoutNV
  {IR(41507, 19), 563}, // 196 CreateTensorViewNV
  {IR(34272, 16), 272}, // 197 CreateUserEvent
  {IR(50109, 30), 805}, // 198 CrossWorkgroupCastToPtrALTERA
  {IR(50139, 29), 805}, // 199 CrossWorkgroupCastToPtrINTEL
  {IR(32972, 5), 192}, // 200 DPdx
  {IR(33018, 11), 198}, // 201 DPdxCoarse
  {IR(32989, 9), 195}, // 202 DPdxFine
  {IR(32977, 5), 193}, // 203 DPdy
  {IR(33029, 11), 199}, // 204 DPdyCoarse
  {IR(32998, 9), 196}, // 205 DPdyFine
  {IR(31179, 9), 64}, // 206 Decorate
  {IR(35002, 11), 305}, // 207 DecorateId
  {IR(42876, 15), 625}, // 208 DecorateString
  {IR(42891, 21), 625}, // 209 DecorateStringGOOGLE
  {IR(31203, 16), 66}, // 210 DecorationGroup
  {IR(13230, 25), 566}, // 211 DemoteToHelperInvocation
  {IR(13255, 28), 566}, // 212 DemoteToHelperInvocationEXT
  {IR(35849, 23), 345}, // 213 DepthAttachmentReadEXT
  {IR(32288, 4), 137}, // 214 Dot
  {IR(39262, 17), 489}, // 215 EmitMeshTasksEXT
  {IR(33077, 17), 203}, // 216 EmitStreamVertex
  {IR(33053, 11), 201}, // 217 EmitVertex
  {IR(41028, 26), 551}, // 218 EndInvocationInterlockEXT
  {IR(33064, 13), 202}, // 219 EndPrimitive
  {IR(33094, 19), 204}, // 220 EndStreamPrimitive
  {IR(34108, 14), 265}, // 221 EnqueueKernel
  {IR(34094, 14), 264}, // 222 EnqueueMarker
  {IR(37763, 24), 430}, // 223 EnqueueNodePayloadsAMDX
  {IR(30022, 11), 13}, // 224 EntryPoint
  {IR(36479, 19), 379}, // 225 ExecuteCallableKHR
  {IR(40610, 18), 535}, // 226 ExecuteCallableNV
  {IR(30065, 14), 14}, // 227 ExecutionMode
  {IR(34986, 16), 304}, // 228 ExecutionModeId
  {IR(42866, 10), 624}, // 229 ExpectKHR
  {IR(29876, 8), 11}, // 230 ExtInst
  {IR(29816, 14), 10}, // 231 ExtInstImport
  {IR(36416, 26), 376}, // 232 ExtInstWithForwardRefsKHR
  {IR(29806, 10), 9}, // 233 Extension
  {IR(37467, 20), 416}, // 234 ExtractSubArrayQCOM
  {IR(32120, 5), 118}, // 235 FAdd
  {IR(31951, 9), 105}, // 236 FConvert
  {IR(32155, 5), 125}, // 237 FDiv
  {IR(52271, 19), 875}, // 238 FDot2MixAcc16VALVE
  {IR(52252, 19), 874}, // 239 FDot2MixAcc32VALVE
  {IR(52290, 19), 876}, // 240 FDot4MixAcc32VALVE
  {IR(32180, 5), 130}, // 241 FMod
  {IR(32140, 5), 122}, // 242 FMul
  {IR(32107, 8), 116}, // 243 FNegate
  {IR(32617, 10), 168}, // 244 FOrdEqual
  {IR(32695, 16), 174}, // 245 FOrdGreaterThan
  {IR(32767, 21), 178}, // 246 FOrdGreaterThanEqual
  {IR(32667, 13), 172}, // 247 FOrdLessThan
  {IR(32729, 18), 176}, // 248 FOrdLessThanEqual
  {IR(32639, 13), 170}, // 249 FOrdNotEqual
  {IR(15375, 14), 808}, // 250 FPGARegALTERA
  {IR(15389, 13), 808}, // 251 FPGARegINTEL
  {IR(32175, 5), 129}, // 252 FRem
  {IR(32130, 5), 120}, // 253 FSub
  {IR(32627, 12), 169}, // 254 FUnordEqual
  {IR(32711, 18), 175}, // 255 FUnordGreaterThan
  {IR(32788, 23), 179}, // 256 FUnordGreaterThanEqual
  {IR(32680, 15), 173}, // 257 FUnordLessThan
  {IR(32747, 20), 177}, // 258 FUnordLessThanEqual
  {IR(32652, 15), 171}, // 259 FUnordNotEqual
  {IR(39420, 38), 494}, // 260 FetchMicroTriangleVertexBarycentricNV
  {IR(39385, 35), 493}, // 261 FetchMicroTriangleVertexPositionNV
  {IR(37812, 29), 432}, // 262 FinishWritingNodePayloadAMDX
  {IR(49823, 15), 797}, // 263 FixedCosALTERA
  {IR(49838, 14), 797}, // 264 FixedCosINTEL
  {IR(49920, 17), 800}, // 265 FixedCosPiALTERA
  {IR(49937, 16), 800}, // 266 FixedCosPiINTEL
  {IR(50021, 15), 803}, // 267 FixedExpALTERA
  {IR(50036, 14), 803}, // 268 FixedExpINTEL
  {IR(49992, 15), 802}, // 269 FixedLogALTERA
  {IR(50007, 14), 802}, // 270 FixedLogINTEL
  {IR(49728, 17), 794}, // 271 FixedRecipALTERA
  {IR(49745, 16), 794}, // 272 FixedRecipINTEL
  {IR(49761, 17), 795}, // 273 FixedRsqrtALTERA
  {IR(49778, 16), 795}, // 274 FixedRsqrtINTEL
  {IR(49794, 15), 796}, // 275 FixedSinALTERA
  {IR(49852, 18), 798}, // 276 FixedSinCosALTERA
  {IR(49870, 17), 798}, // 277 FixedSinCosINTEL
  {IR(49953, 20), 801}, // 278 FixedSinCosPiALTERA
  {IR(49973, 19), 801}, // 279 FixedSinCosPiINTEL
  {IR(49809, 14), 796}, // 280 FixedSinINTEL
  {IR(49887, 17), 799}, // 281 FixedSinPiALTERA
  {IR(49904, 16), 799}, // 282 FixedSinPiINTEL
  {IR(49697, 16), 793}, // 283 FixedSqrtALTERA
  {IR(49713, 15), 793}, // 284 FixedSqrtINTEL
  {IR(36333, 7), 370}, // 285 FmaKHR
  {IR(37708, 17), 427}, // 286 FragmentFetchAMD
  {IR(37687, 21), 426}, // 287 FragmentMaskFetchAMD
  {IR(38112, 10), 448}, // 288 FreezeKHR
  {IR(28855, 9), 48}, // 289 Function
  {IR(30928, 13), 51}, // 290 FunctionCall
  {IR(30916, 12), 50}, // 291 FunctionEnd
  {IR(30898, 18), 49}, // 292 FunctionParameter
  {IR(42771, 25), 617}, // 293 FunctionPointerCallINTEL
  {IR(32982, 7), 194}, // 294 Fwidth
  {IR(33040, 13), 200}, // 295 FwidthCoarse
  {IR(33007, 11), 197}, // 296 FwidthFine
  {IR(32049, 17), 112}, // 297 GenericCastToPtr
  {IR(32066, 25), 113}, // 298 GenericCastToPtrExplicit
  {IR(31105, 23), 62}, // 299 GenericPtrMemSemantics
  {IR(34346, 16), 276}, // 300 GetDefaultQueue
  {IR(34851, 35), 298}, // 301 GetKernelLocalSizeForSubgroupCount
  {IR(34886, 25), 299}, // 302 GetKernelMaxNumSubgroups
  {IR(34152, 32), 267}, // 303 GetKernelNDrangeMaxSubGroupSize
  {IR(34122, 30), 266}, // 304 GetKernelNDrangeSubGroupCount
  {IR(34207, 40), 269}, // 305 GetKernelPreferredWorkGroupSizeMultiple
  {IR(34184, 23), 268}, // 306 GetKernelWorkGroupSize
  {IR(33978, 18), 259}, // 307 GetMaxPipePackets
  {IR(33960, 18), 258}, // 308 GetNumPipePackets
  {IR(10433, 9), 353}, // 309 GraphARM
  {IR(36001, 17), 351}, // 310 GraphConstantARM
  {IR(36069, 12), 356}, // 311 GraphEndARM
  {IR(36018, 19), 352}, // 312 GraphEntryPointARM
  {IR(36037, 14), 354}, // 313 GraphInputARM
  {IR(36051, 18), 355}, // 314 GraphSetOutputARM
  {IR(33665, 9), 238}, // 315 GroupAll
  {IR(33674, 9), 239}, // 316 GroupAny
  {IR(33634, 15), 236}, // 317 GroupAsyncCopy
  {IR(51998, 19), 862}, // 318 GroupBitwiseAndKHR
  {IR(52017, 18), 863}, // 319 GroupBitwiseOrKHR
  {IR(52035, 19), 864}, // 320 GroupBitwiseXorKHR
  {IR(33683, 15), 240}, // 321 GroupBroadcast
  {IR(34053, 20), 262}, // 322 GroupCommitReadPipe
  {IR(34073, 21), 263}, // 323 GroupCommitWritePipe
  {IR(31219, 14), 67}, // 324 GroupDecorate
  {IR(33741, 10), 242}, // 325 GroupFAdd
  {IR(37526, 23), 419}, // 326 GroupFAddNonUniformAMD
  {IR(33781, 10), 246}, // 327 GroupFMax
  {IR(37618, 23), 423}, // 328 GroupFMaxNonUniformAMD
  {IR(33751, 10), 243}, // 329 GroupFMin
  {IR(37549, 23), 420}, // 330 GroupFMinNonUniformAMD
  {IR(51985, 13), 861}, // 331 GroupFMulKHR
  {IR(33731, 10), 241}, // 332 GroupIAdd
  {IR(37503, 23), 418}, // 333 GroupIAddNonUniformAMD
  {IR(51972, 13), 860}, // 334 GroupIMulKHR
  {IR(52054, 19), 865}, // 335 GroupLogicalAndKHR
  {IR(52073, 18), 866}, // 336 GroupLogicalOrKHR
  {IR(52091, 19), 867}, // 337 GroupLogicalXorKHR
  {IR(31278, 20), 68}, // 338 GroupMemberDecorate
  {IR(35034, 19), 307}, // 339 GroupNonUniformAll
  {IR(35072, 24), 309}, // 340 GroupNonUniformAllEqual
  {IR(35053, 19), 308}, // 341 GroupNonUniformAny
  {IR(6960, 22), 312}, // 342 GroupNonUniformBallot
  {IR(35212, 30), 315}, // 343 GroupNonUniformBallotBitCount
  {IR(35180, 32), 314}, // 344 GroupNonUniformBallotBitExtract
  {IR(35242, 29), 316}, // 345 GroupNonUniformBallotFindLSB
  {IR(35271, 29), 317}, // 346 GroupNonUniformBallotFindMSB
  {IR(35578, 26), 332}, // 347 GroupNonUniformBitwiseAnd
  {IR(35604, 25), 333}, // 348 GroupNonUniformBitwiseOr
  {IR(35629, 26), 334}, // 349 GroupNonUniformBitwiseXor
  {IR(35096, 25), 310}, // 350 GroupNonUniformBroadcast
  {IR(35121, 30), 311}, // 351 GroupNonUniformBroadcastFirst
  {IR(35013, 21), 306}, // 352 GroupNonUniformElect
  {IR(35398, 20), 323}, // 353 GroupNonUniformFAdd
  {IR(35558, 20), 331}, // 354 GroupNonUniformFMax
  {IR(35498, 20), 328}, // 355 GroupNonUniformFMin
  {IR(35438, 20), 325}, // 356 GroupNonUniformFMul
  {IR(35378, 20), 322}, // 357 GroupNonUniformIAdd
  {IR(35418, 20), 324}, // 358 GroupNonUniformIMul
  {IR(35151, 29), 313}, // 359 GroupNonUniformInverseBallot
  {IR(35655, 26), 335}, // 360 GroupNonUniformLogicalAnd
  {IR(35681, 25), 336}, // 361 GroupNonUniformLogicalOr
  {IR(35706, 26), 337}, // 362 GroupNonUniformLogicalXor
  {IR(39297, 28), 491}, // 363 GroupNonUniformPartitionEXT
  {IR(39325, 27), 491}, // 364 GroupNonUniformPartitionNV
  {IR(37933, 26), 437}, // 365 GroupNonUniformQuadAllKHR
  {IR(37959, 26), 438}, // 366 GroupNonUniformQuadAnyKHR
  {IR(35732, 29), 338}, // 367 GroupNonUniformQuadBroadcast
  {IR(35761, 24), 339}, // 368 GroupNonUniformQuadSwap
  {IR(15614, 25), 374}, // 369 GroupNonUniformRotateKHR
  {IR(35518, 20), 329}, // 370 GroupNonUniformSMax
  {IR(35458, 20), 326}, // 371 GroupNonUniformSMin
  {IR(10142, 23), 318}, // 372 GroupNonUniformShuffle
  {IR(35351, 27), 321}, // 373 GroupNonUniformShuffleDown
  {IR(35326, 25), 320}, // 374 GroupNonUniformShuffleUp
  {IR(35300, 26), 319}, // 375 GroupNonUniformShuffleXor
  {IR(35538, 20), 330}, // 376 GroupNonUniformUMax
  {IR(35478, 20), 327}, // 377 GroupNonUniformUMin
  {IR(33996, 28), 260}, // 378 GroupReserveReadPipePackets
  {IR(34024, 29), 261}, // 379 GroupReserveWritePipePackets
  {IR(33801, 10), 248}, // 380 GroupSMax
  {IR(37664, 23), 425}, // 381 GroupSMaxNonUniformAMD
  {IR(33771, 10), 245}, // 382 GroupSMin
  {IR(37595, 23), 422}, // 383 GroupSMinNonUniformAMD
  {IR(33791, 10), 247}, // 384 GroupUMax
  {IR(37641, 23), 424}, // 385 GroupUMaxNonUniformAMD
  {IR(33761, 10), 244}, // 386 GroupUMin
  {IR(37572, 23), 421}, // 387 GroupUMinNonUniformAMD
  {IR(33649, 16), 237}, // 388 GroupWaitEvents
  {IR(39954, 26), 512}, // 389 HitObjectExecuteShaderEXT
  {IR(38557, 25), 464}, // 390 HitObjectExecuteShaderNV
  {IR(40007, 26), 514}, // 391 HitObjectGetAttributesEXT
  {IR(38608, 25), 466}, // 392 HitObjectGetAttributesNV
  {IR(40686, 24), 537}, // 393 HitObjectGetClusterIdNV
  {IR(39980, 27), 513}, // 394 HitObjectGetCurrentTimeEXT
  {IR(38582, 26), 465}, // 395 HitObjectGetCurrentTimeNV
  {IR(40086, 29), 517}, // 396 HitObjectGetGeometryIndexEXT
  {IR(38684, 28), 469}, // 397 HitObjectGetGeometryIndexNV
  {IR(40033, 23), 515}, // 398 HitObjectGetHitKindEXT
  {IR(38633, 22), 467}, // 399 HitObjectGetHitKindNV
  {IR(40141, 35), 519}, // 400 HitObjectGetInstanceCustomIndexEXT
  {IR(38737, 34), 471}, // 401 HitObjectGetInstanceCustomIndexNV
  {IR(40115, 26), 518}, // 402 HitObjectGetInstanceIdEXT
  {IR(38712, 25), 470}, // 403 HitObjectGetInstanceIdNV
  {IR(39587, 51), 500}, // 404 HitObjectGetIntersectionTriangleVertexPositionsEXT
  {IR(42138, 27), 586}, // 405 HitObjectGetLSSPositionsNV
  {IR(42165, 23), 587}, // 406 HitObjectGetLSSRadiiNV
  {IR(40207, 34), 521}, // 407 HitObjectGetObjectRayDirectionEXT
  {IR(38269, 33), 454}, // 408 HitObjectGetObjectRayDirectionNV
  {IR(40176, 31), 520}, // 409 HitObjectGetObjectRayOriginEXT
  {IR(38302, 30), 455}, // 410 HitObjectGetObjectRayOriginNV
  {IR(40304, 29), 524}, // 411 HitObjectGetObjectToWorldEXT
  {IR(38241, 28), 453}, // 412 HitObjectGetObjectToWorldNV
  {IR(40056, 30), 516}, // 413 HitObjectGetPrimitiveIndexEXT
  {IR(38655, 29), 468}, // 414 HitObjectGetPrimitiveIndexNV
  {IR(39638, 24), 501}, // 415 HitObjectGetRayFlagsEXT
  {IR(40362, 23), 526}, // 416 HitObjectGetRayTMaxEXT
  {IR(38832, 22), 474}, // 417 HitObjectGetRayTMaxNV
  {IR(40710, 23), 538}, // 418 HitObjectGetRayTMinEXT
  {IR(38854, 22), 475}, // 419 HitObjectGetRayTMinNV
  {IR(40733, 45), 539}, // 420 HitObjectGetShaderBindingTableRecordIndexEXT
  {IR(38397, 44), 458}, // 421 HitObjectGetShaderBindingTableRecordIndexNV
  {IR(40778, 40), 540}, // 422 HitObjectGetShaderRecordBufferHandleEXT
  {IR(38358, 39), 457}, // 423 HitObjectGetShaderRecordBufferHandleNV
  {IR(42082, 29), 584}, // 424 HitObjectGetSpherePositionNV
  {IR(42111, 27), 585}, // 425 HitObjectGetSphereRadiusNV
  {IR(40241, 33), 522}, // 426 HitObjectGetWorldRayDirectionEXT
  {IR(38771, 32), 472}, // 427 HitObjectGetWorldRayDirectionNV
  {IR(40274, 30), 523}, // 428 HitObjectGetWorldRayOriginEXT
  {IR(38803, 29), 473}, // 429 HitObjectGetWorldRayOriginNV
  {IR(40333, 29), 525}, // 430 HitObjectGetWorldToObjectEXT
  {IR(38213, 28), 452}, // 431 HitObjectGetWorldToObjectNV
  {IR(40818, 20), 541}, // 432 HitObjectIsEmptyEXT
  {IR(38876, 19), 476}, // 433 HitObjectIsEmptyNV
  {IR(40838, 18), 542}, // 434 HitObjectIsHitEXT
  {IR(38895, 17), 477}, // 435 HitObjectIsHitNV
  {IR(42211, 20), 589}, // 436 HitObjectIsLSSHitNV
  {IR(40856, 19), 543}, // 437 HitObjectIsMissEXT
  {IR(38912, 18), 478}, // 438 HitObjectIsMissNV
  {IR(42188, 23), 588}, // 439 HitObjectIsSphereHitNV
  {IR(39930, 24), 511}, // 440 HitObjectRecordEmptyEXT
  {IR(38441, 23), 459}, // 441 HitObjectRecordEmptyNV
  {IR(39507, 28), 497}, // 442 HitObjectRecordFromQueryEXT
  {IR(38122, 27), 449}, // 443 HitObjectRecordHitMotionNV
  {IR(38484, 21), 461}, // 444 HitObjectRecordHitNV
  {IR(38149, 36), 450}, // 445 HitObjectRecordHitWithIndexMotionNV
  {IR(38505, 30), 462}, // 446 HitObjectRecordHitWithIndexNV
  {IR(39535, 23), 498}, // 447 HitObjectRecordMissEXT
  {IR(39558, 29), 499}, // 448 HitObjectRecordMissMotionEXT
  {IR(38185, 28), 451}, // 449 HitObjectRecordMissMotionNV
  {IR(38535, 22), 463}, // 450 HitObjectRecordMissNV
  {IR(39707, 33), 503}, // 451 HitObjectReorderExecuteShaderEXT
  {IR(39662, 45), 502}, // 452 HitObjectSetShaderBindingTableRecordIndexEXT
  {IR(39772, 38), 505}, // 453 HitObjectTraceMotionReorderExecuteEXT
  {IR(39882, 21), 509}, // 454 HitObjectTraceRayEXT
  {IR(39903, 27), 510}, // 455 HitObjectTraceRayMotionEXT
  {IR(38332, 26), 456}, // 456 HitObjectTraceRayMotionNV
  {IR(38464, 20), 460}, // 457 HitObjectTraceRayNV
  {IR(39740, 32), 504}, // 458 HitObjectTraceReorderExecuteEXT
  {IR(32115, 5), 117}, // 459 IAdd
  {IR(32292, 10), 138}, // 460 IAddCarry
  {IR(42590, 13), 606}, // 461 IAddSatINTEL
  {IR(42616, 14), 608}, // 462 IAverageINTEL
  {IR(42644, 21), 610}, // 463 IAverageRoundedINTEL
  {IR(32488, 7), 158}, // 464 IEqual
  {IR(32135, 5), 121}, // 465 IMul
  {IR(42712, 15), 614}, // 466 IMul32x16INTEL
  {IR(32495, 10), 159}, // 467 INotEqual
  {IR(32125, 5), 119}, // 468 ISub
  {IR(32302, 11), 139}, // 469 ISubBorrow
  {IR(42686, 13), 612}, // 470 ISubSatINTEL
  {IR(36533, 22), 381}, // 471 IgnoreIntersectionKHR
  {IR(40428, 21), 528}, // 472 IgnoreIntersectionNV
  {IR(28899, 6), 91}, // 473 Image
  {IR(37380, 29), 412}, // 474 ImageBlockMatchGatherSADQCOM
  {IR(37351, 29), 411}, // 475 ImageBlockMatchGatherSSDQCOM
  {IR(37253, 23), 407}, // 476 ImageBlockMatchSADQCOM
  {IR(37230, 23), 406}, // 477 ImageBlockMatchSSDQCOM
  {IR(37322, 29), 410}, // 478 ImageBlockMatchWindowSADQCOM
  {IR(37293, 29), 409}, // 479 ImageBlockMatchWindowSSDQCOM
  {IR(37211, 19), 405}, // 480 ImageBoxFilterQCOM
  {IR(31733, 16), 88}, // 481 ImageDrefGather
  {IR(31710, 11), 86}, // 482 ImageFetch
  {IR(31721, 12), 87}, // 483 ImageGather
  {IR(37487, 16), 417}, // 484 ImageGatherQCOM
  {IR(31770, 17), 92}, // 485 ImageQueryFormat
  {IR(31850, 17), 97}, // 486 ImageQueryLevels
  {IR(31836, 14), 96}, // 487 ImageQueryLod
  {IR(31787, 16), 93}, // 488 ImageQueryOrder
  {IR(31867, 18), 98}, // 489 ImageQuerySamples
  {IR(31821, 15), 95}, // 490 ImageQuerySize
  {IR(31803, 18), 94}, // 491 ImageQuerySizeLod
  {IR(31749, 10), 89}, // 492 ImageRead
  {IR(31567, 27), 81}, // 493 ImageSampleDrefExplicitLod
  {IR(31540, 27), 80}, // 494 ImageSampleDrefImplicitLod
  {IR(31517, 23), 79}, // 495 ImageSampleExplicitLod
  {IR(38999, 23), 482}, // 496 ImageSampleFootprintNV
  {IR(31471, 23), 78}, // 497 ImageSampleImplicitLod
  {IR(31679, 31), 85}, // 498 ImageSampleProjDrefExplicitLod
  {IR(31648, 31), 84}, // 499 ImageSampleProjDrefImplicitLod
  {IR(31621, 27), 83}, // 500 ImageSampleProjExplicitLod
  {IR(31594, 27), 82}, // 501 ImageSampleProjImplicitLod
  {IR(37187, 24), 404}, // 502 ImageSampleWeightedQCOM
  {IR(34674, 22), 288}, // 503 ImageSparseDrefGather
  {IR(34639, 17), 286}, // 504 ImageSparseFetch
  {IR(34656, 18), 287}, // 505 ImageSparseGather
  {IR(34766, 16), 293}, // 506 ImageSparseRead
  {IR(34466, 33), 281}, // 507 ImageSparseSampleDrefExplicitLod
  {IR(34433, 33), 280}, // 508 ImageSparseSampleDrefImplicitLod
  {IR(34404, 29), 279}, // 509 ImageSparseSampleExplicitLod
  {IR(34375, 29), 278}, // 510 ImageSparseSampleImplicitLod
  {IR(34602, 37), 285}, // 511 ImageSparseSampleProjDrefExplicitLod
  {IR(34565, 37), 284}, // 512 ImageSparseSampleProjDrefImplicitLod
  {IR(34532, 33), 283}, // 513 ImageSparseSampleProjExplicitLod
  {IR(34499, 33), 282}, // 514 ImageSparseSampleProjImplicitLod
  {IR(34696, 26), 289}, // 515 ImageSparseTexelsResident
  {IR(30950, 18), 53}, // 516 ImageTexelPointer
  {IR(31759, 11), 90}, // 517 ImageWrite
  {IR(31058, 20), 59}, // 518 InBoundsAccessChain
  {IR(31128, 23), 63}, // 519 InBoundsPtrAccessChain
  {IR(32359, 9), 146}, // 520 IsFinite
  {IR(41573, 22), 567}, // 521 IsHelperInvocationEXT
  {IR(32353, 6), 145}, // 522 IsInf
  {IR(32347, 6), 144}, // 523 IsNan
  {IR(37868, 23), 434}, // 524 IsNodePayloadValidAMDX
  {IR(32368, 9), 147}, // 525 IsNormal
  {IR(34288, 13), 273}, // 526 IsValidEvent
  {IR(33943, 17), 257}, // 527 IsValidReserveId
  {IR(33571, 5), 230}, // 528 Kill
  {IR(33488, 6), 226}, // 529 Label
  {IR(32388, 14), 149}, // 530 LessOrGreater
  {IR(33607, 14), 234}, // 531 LifetimeStart
  {IR(33621, 13), 235}, // 532 LifetimeStop
  {IR(29801, 5), 8}, // 533 Line
  {IR(31008, 5), 54}, // 534 Load
  {IR(32459, 11), 155}, // 535 LogicalAnd
  {IR(32420, 13), 152}, // 536 LogicalEqual
  {IR(32470, 11), 156}, // 537 LogicalNot
  {IR(32433, 16), 153}, // 538 LogicalNotEqual
  {IR(32449, 10), 154}, // 539 LogicalOr
  {IR(49615, 17), 789}, // 540 LoopControlINTEL
  {IR(33428, 10), 224}, // 541 LoopMerge
  {IR(52128, 18), 869}, // 542 MaskedGatherINTEL
  {IR(52146, 19), 870}, // 543 MaskedScatterINTEL
  {IR(32257, 18), 135}, // 544 MatrixTimesMatrix
  {IR(32203, 18), 132}, // 545 MatrixTimesScalar
  {IR(32239, 18), 134}, // 546 MatrixTimesVector
  {IR(31188, 15), 65}, // 547 MemberDecorate
  {IR(38044, 20), 443}, // 548 MemberDecorateIdEXT
  {IR(42912, 21), 626}, // 549 MemberDecorateString
  {IR(42933, 27), 626}, // 550 MemberDecorateStringGOOGLE
  {IR(29783, 11), 6}, // 551 MemberName
  {IR(33165, 14), 206}, // 552 MemoryBarrier
  {IR(29948, 12), 12}, // 553 MemoryModel
  {IR(34951, 19), 302}, // 554 MemoryNamedBarrier
  {IR(34970, 16), 303}, // 555 ModuleProcessed
  {IR(29778, 5), 5}, // 556 Name
  {IR(34928, 23), 301}, // 557 NamedBarrierInitialize
  {IR(34722, 7), 290}, // 558 NoLine
  {IR(37841, 27), 433}, // 559 NodePayloadArrayLengthAMDX
  {IR(29574, 4), 0}, // 560 Nop
  {IR(32899, 4), 186}, // 561 Not
  {IR(32402, 8), 150}, // 562 Ordered
  {IR(32275, 13), 136}, // 563 OuterProduct
  {IR(33394, 4), 223}, // 564 Phi
  {IR(38102, 10), 447}, // 565 PoisonKHR
  {IR(51931, 20), 858}, // 566 PredicatedLoadINTEL
  {IR(51951, 21), 859}, // 567 PredicatedStoreINTEL
  {IR(31078, 15), 60}, // 568 PtrAccessChain
  {IR(50050, 30), 804}, // 569 PtrCastToCrossWorkgroupALTERA
  {IR(50080, 29), 804}, // 570 PtrCastToCrossWorkgroupINTEL
  {IR(32032, 17), 111}, // 571 PtrCastToGeneric
  {IR(35818, 8), 343}, // 572 PtrDiff
  {IR(35797, 9), 341}, // 573 PtrEqual
  {IR(35806, 12), 342}, // 574 PtrNotEqual
  {IR(31960, 14), 106}, // 575 QuantizeToF16
  {IR(41878, 17), 578}, // 576 RawAccessChainNV
  {IR(37106, 31), 401}, // 577 RayQueryConfirmIntersectionKHR
  {IR(37074, 32), 400}, // 578 RayQueryGenerateIntersectionKHR
  {IR(40663, 23), 536}, // 579 RayQueryGetClusterIdNV
  {IR(50562, 39), 817}, // 580 RayQueryGetIntersectionBarycentricsKHR
  {IR(50637, 46), 819}, // 581 RayQueryGetIntersectionCandidateAABBOpaqueKHR
  {IR(40628, 35), 536}, // 582 RayQueryGetIntersectionClusterIdNV
  {IR(50601, 36), 818}, // 583 RayQueryGetIntersectionFrontFaceKHR
  {IR(50481, 40), 815}, // 584 RayQueryGetIntersectionGeometryIndexKHR
  {IR(50333, 46), 812}, // 585 RayQueryGetIntersectionInstanceCustomIndexKHR
  {IR(50379, 37), 813}, // 586 RayQueryGetIntersectionInstanceIdKHR
  {IR(50416, 65), 814}, // 587 RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR
  {IR(42045, 37), 583}, // 588 RayQueryGetIntersectionLSSHitValueNV
  {IR(41973, 38), 581}, // 589 RayQueryGetIntersectionLSSPositionsNV
  {IR(42011, 34), 582}, // 590 RayQueryGetIntersectionLSSRadiiNV
  {IR(50683, 45), 820}, // 591 RayQueryGetIntersectionObjectRayDirectionKHR
  {IR(50728, 42), 821}, // 592 RayQueryGetIntersectionObjectRayOriginKHR
  {IR(50831, 40), 824}, // 593 RayQueryGetIntersectionObjectToWorldKHR
  {IR(50521, 41), 816}, // 594 RayQueryGetIntersectionPrimitiveIndexKHR
  {IR(41895, 40), 579}, // 595 RayQueryGetIntersectionSpherePositionNV
  {IR(41935, 38), 580}, // 596 RayQueryGetIntersectionSphereRadiusNV
  {IR(50305, 28), 811}, // 597 RayQueryGetIntersectionTKHR
  {IR(40503, 50), 533}, // 598 RayQueryGetIntersectionTriangleVertexPositionsKHR
  {IR(37156, 31), 403}, // 599 RayQueryGetIntersectionTypeKHR
  {IR(50871, 40), 825}, // 600 RayQueryGetIntersectionWorldToObjectKHR
  {IR(50282, 23), 810}, // 601 RayQueryGetRayFlagsKHR
  {IR(50260, 22), 809}, // 602 RayQueryGetRayTMinKHR
  {IR(50770, 32), 822}, // 603 RayQueryGetWorldRayDirectionKHR
  {IR(50802, 29), 823}, // 604 RayQueryGetWorldRayOriginKHR
  {IR(37031, 22), 398}, // 605 RayQueryInitializeKHR
  {IR(42253, 19), 591}, // 606 RayQueryIsLSSHitNV
  {IR(42231, 22), 590}, // 607 RayQueryIsSphereHitNV
  {IR(37137, 19), 402}, // 608 RayQueryProceedKHR
  {IR(37053, 21), 399}, // 609 RayQueryTerminateKHR
  {IR(37725, 13), 428}, // 610 ReadClockKHR
  {IR(33811, 9), 249}, // 611 ReadPipe
  {IR(50168, 23), 806}, // 612 ReadPipeBlockingALTERA
  {IR(50191, 22), 806}, // 613 ReadPipeBlockingINTEL
  {IR(34259, 13), 271}, // 614 ReleaseEvent
  {IR(39827, 25), 507}, // 615 ReorderThreadWithHintEXT
  {IR(38959, 24), 480}, // 616 ReorderThreadWithHintNV
  {IR(39852, 30), 508}, // 617 ReorderThreadWithHitObjectEXT
  {IR(38930, 29), 479}, // 618 ReorderThreadWithHitObjectNV
  {IR(40385, 22), 527}, // 619 ReportIntersectionKHR
  {IR(40407, 21), 527}, // 620 ReportIntersectionNV
  {IR(33865, 23), 253}, // 621 ReserveReadPipePackets
  {IR(33888, 24), 254}, // 622 ReserveWritePipePackets
  {IR(33830, 17), 251}, // 623 ReservedReadPipe
  {IR(33847, 18), 252}, // 624 ReservedWritePipe
  {IR(48142, 19), 747}, // 625 RestoreMemoryINTEL
  {IR(34247, 12), 270}, // 626 RetainEvent
  {IR(33576, 7), 231}, // 627 Return
  {IR(33583, 12), 232}, // 628 ReturnValue
  {IR(52110, 18), 868}, // 629 RoundFToTF32INTEL
  {IR(31942, 9), 104}, // 630 SConvert
  {IR(32150, 5), 124}, // 631 SDiv
  {IR(36618, 5), 383}, // 632 SDot
  {IR(36659, 11), 386}, // 633 SDotAccSat
  {IR(36670, 14), 386}, // 634 SDotAccSatKHR
  {IR(36623, 8), 383}, // 635 SDotKHR
  {IR(32518, 13), 161}, // 636 SGreaterThan
  {IR(32549, 18), 163}, // 637 SGreaterThanEqual
  {IR(32577, 10), 165}, // 638 SLessThan
  {IR(32602, 15), 167}, // 639 SLessThanEqual
  {IR(32170, 5), 128}, // 640 SMod
  {IR(32326, 13), 141}, // 641 SMulExtended
  {IR(32099, 8), 115}, // 642 SNegate
  {IR(32165, 5), 127}, // 643 SRem
  {IR(36644, 6), 385}, // 644 SUDot
  {IR(36709, 12), 388}, // 645 SUDotAccSat
  {IR(36721, 15), 388}, // 646 SUDotAccSatKHR
  {IR(36650, 9), 385}, // 647 SUDotKHR
  {IR(31426, 13), 77}, // 648 SampledImage
  {IR(41797, 29), 577}, // 649 SamplerImageAddressingModeNV
  {IR(31988, 15), 108}, // 650 SatConvertSToU
  {IR(32003, 15), 109}, // 651 SatConvertUToS
  {IR(48126, 16), 746}, // 652 SaveMemoryINTEL
  {IR(32481, 7), 157}, // 653 Select
  {IR(33473, 15), 225}, // 654 SelectionMerge
  {IR(39279, 18), 490}, // 655 SetMeshOutputsEXT
  {IR(34301, 19), 274}, // 656 SetUserEventStatus
  {IR(32850, 17), 182}, // 657 ShiftLeftLogical
  {IR(32829, 21), 181}, // 658 ShiftRightArithmetic
  {IR(32811, 18), 180}, // 659 ShiftRightLogical
  {IR(32377, 11), 148}, // 660 SignBitSet
  {IR(34782, 7), 294}, // 661 SizeOf
  {IR(29755, 7), 3}, // 662 Source
  {IR(29636, 16), 2}, // 663 SourceContinued
  {IR(29762, 16), 4}, // 664 SourceExtension
  {IR(30773, 13), 45}, // 665 SpecConstant
  {IR(51807, 30), 855}, // 666 SpecConstantArchitectureINTEL
  {IR(51874, 30), 856}, // 667 SpecConstantCapabilitiesINTEL
  {IR(30786, 22), 46}, // 668 SpecConstantComposite
  {IR(51005, 36), 830}, // 669 SpecConstantCompositeContinuedINTEL
  {IR(36950, 34), 395}, // 670 SpecConstantCompositeReplicateEXT
  {IR(38082, 20), 446}, // 671 SpecConstantDataKHR
  {IR(30755, 18), 44}, // 672 SpecConstantFalse
  {IR(30849, 15), 47}, // 673 SpecConstantOp
  {IR(37910, 23), 436}, // 674 SpecConstantStringAMDX
  {IR(51783, 24), 854}, // 675 SpecConstantTargetINTEL
  {IR(30738, 17), 43}, // 676 SpecConstantTrue
  {IR(35872, 25), 346}, // 677 StencilAttachmentReadEXT
  {IR(31013, 6), 55}, // 678 Store
  {IR(29794, 7), 7}, // 679 String
  {IR(51472, 25), 843}, // 680 Subgroup2DBlockLoadINTEL
  {IR(51497, 34), 844}, // 681 Subgroup2DBlockLoadTransformINTEL
  {IR(51531, 34), 845}, // 682 Subgroup2DBlockLoadTransposeINTEL
  {IR(51565, 29), 846}, // 683 Subgroup2DBlockPrefetchINTEL
  {IR(51594, 26), 847}, // 684 Subgroup2DBlockStoreINTEL
  {IR(36370, 20), 373}, // 685 SubgroupAllEqualKHR
  {IR(36340, 15), 371}, // 686 SubgroupAllKHR
  {IR(36355, 15), 372}, // 687 SubgroupAnyKHR
  {IR(46644, 30), 710}, // 688 SubgroupAvcBmeInitializeINTEL
  {IR(46614, 30), 709}, // 689 SubgroupAvcFmeInitializeINTEL
  {IR(45049, 35), 679}, // 690 SubgroupAvcImeAdjustRefOffsetINTEL
  {IR(45084, 39), 680}, // 691 SubgroupAvcImeConvertToMcePayloadINTEL
  {IR(45725, 38), 693}, // 692 SubgroupAvcImeConvertToMceResultINTEL
  {IR(45348, 45), 686}, // 693 SubgroupAvcImeEvaluateWithDualReferenceINTEL
  {IR(45448, 53), 688}, // 694 SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL
  {IR(45669, 56), 692}, // 695 SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL
  {IR(45557, 54), 690}, // 696 SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL
  {IR(45301, 47), 685}, // 697 SubgroupAvcImeEvaluateWithSingleReferenceINTEL
  {IR(45393, 55), 687}, // 698 SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL
  {IR(45611, 58), 691}, // 699 SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL
  {IR(45501, 56), 689}, // 700 SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL
  {IR(46357, 36), 704}, // 701 SubgroupAvcImeGetBorderReachedINTEL
  {IR(45809, 44), 695}, // 702 SubgroupAvcImeGetDualReferenceStreaminINTEL
  {IR(45763, 46), 694}, // 703 SubgroupAvcImeGetSingleReferenceStreaminINTEL
  {IR(46224, 66), 702}, // 704 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL
  {IR(46156, 68), 701}, // 705 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL
  {IR(46290, 67), 703}, // 706 SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL
  {IR(46019, 68), 699}, // 707 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL
  {IR(45949, 70), 698}, // 708 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL
  {IR(46087, 69), 700}, // 709 SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL
  {IR(46393, 48), 705}, // 710 SubgroupAvcImeGetTruncatedSearchIndicationINTEL
  {IR(46441, 59), 706}, // 711 SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL
  {IR(46558, 56), 708}, // 712 SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL
  {IR(46500, 58), 707}, // 713 SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL
  {IR(44912, 30), 675}, // 714 SubgroupAvcImeInitializeINTEL
  {IR(45016, 33), 678}, // 715 SubgroupAvcImeRefWindowSizeINTEL
  {IR(44980, 36), 677}, // 716 SubgroupAvcImeSetDualReferenceINTEL
  {IR(45213, 54), 683}, // 717 SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL
  {IR(45123, 43), 681}, // 718 SubgroupAvcImeSetMaxMotionVectorCountINTEL
  {IR(44942, 38), 676}, // 719 SubgroupAvcImeSetSingleReferenceINTEL
  {IR(45166, 47), 682}, // 720 SubgroupAvcImeSetUnidirectionalMixDisableINTEL
  {IR(45267, 34), 684}, // 721 SubgroupAvcImeSetWeightedSadINTEL
  {IR(45902, 47), 697}, // 722 SubgroupAvcImeStripDualReferenceStreamoutINTEL
  {IR(45853, 49), 696}, // 723 SubgroupAvcImeStripSingleReferenceStreamoutINTEL
  {IR(44302, 39), 660}, // 724 SubgroupAvcMceConvertToImePayloadINTEL
  {IR(44341, 38), 661}, // 725 SubgroupAvcMceConvertToImeResultINTEL
  {IR(44379, 39), 662}, // 726 SubgroupAvcMceConvertToRefPayloadINTEL
  {IR(44418, 38), 663}, // 727 SubgroupAvcMceConvertToRefResultINTEL
  {IR(44456, 39), 664}, // 728 SubgroupAvcMceConvertToSicPayloadINTEL
  {IR(44495, 38), 665}, // 729 SubgroupAvcMceConvertToSicResultINTEL
  {IR(44608, 43), 668}, // 730 SubgroupAvcMceGetBestInterDistortionsINTEL
  {IR(43740, 50), 649}, // 731 SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL
  {IR(43338, 60), 641}, // 732 SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL
  {IR(43538, 51), 645}, // 733 SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL
  {IR(43684, 56), 648}, // 734 SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL
  {IR(43451, 47), 643}, // 735 SubgroupAvcMceGetDefaultInterShapePenaltyINTEL
  {IR(44039, 56), 655}, // 736 SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL
  {IR(43938, 50), 653}, // 737 SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL
  {IR(43633, 51), 647}, // 738 SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL
  {IR(43842, 49), 651}, // 739 SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL
  {IR(43790, 52), 650}, // 740 SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL
  {IR(43988, 51), 654}, // 741 SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL
  {IR(44727, 38), 671}, // 742 SubgroupAvcMceGetInterDirectionsINTEL
  {IR(44569, 39), 667}, // 743 SubgroupAvcMceGetInterDistortionsINTEL
  {IR(44651, 38), 669}, // 744 SubgroupAvcMceGetInterMajorShapeINTEL
  {IR(44689, 38), 670}, // 745 SubgroupAvcMceGetInterMinorShapeINTEL
  {IR(44765, 45), 672}, // 746 SubgroupAvcMceGetInterMotionVectorCountINTEL
  {IR(44810, 40), 673}, // 747 SubgroupAvcMceGetInterReferenceIdsINTEL
  {IR(44850, 62), 674}, // 748 SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL
  {IR(44533, 36), 666}, // 749 SubgroupAvcMceGetMotionVectorsINTEL
  {IR(44095, 33), 656}, // 750 SubgroupAvcMceSetAcOnlyHaarINTEL
  {IR(44241, 61), 659}, // 751 SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL
  {IR(43398, 53), 642}, // 752 SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL
  {IR(43589, 44), 646}, // 753 SubgroupAvcMceSetInterDirectionPenaltyINTEL
  {IR(43498, 40), 644}, // 754 SubgroupAvcMceSetInterShapePenaltyINTEL
  {IR(43891, 47), 652}, // 755 SubgroupAvcMceSetMotionVectorCostFunctionINTEL
  {IR(44180, 61), 658}, // 756 SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL
  {IR(44128, 52), 657}, // 757 SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL
  {IR(46674, 39), 711}, // 758 SubgroupAvcRefConvertToMcePayloadINTEL
  {IR(46996, 38), 718}, // 759 SubgroupAvcRefConvertToMceResultINTEL
  {IR(46849, 45), 715}, // 760 SubgroupAvcRefEvaluateWithDualReferenceINTEL
  {IR(46894, 46), 716}, // 761 SubgroupAvcRefEvaluateWithMultiReferenceINTEL
  {IR(46940, 56), 717}, // 762 SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL
  {IR(46802, 47), 714}, // 763 SubgroupAvcRefEvaluateWithSingleReferenceINTEL
  {IR(46713, 46), 712}, // 764 SubgroupAvcRefSetBidirectionalMixDisableINTEL
  {IR(46759, 43), 713}, // 765 SubgroupAvcRefSetBilinearFilterEnableINTEL
  {IR(47132, 42), 722}, // 766 SubgroupAvcSicConfigureIpeLumaChromaINTEL
  {IR(47096, 36), 721}, // 767 SubgroupAvcSicConfigureIpeLumaINTEL
  {IR(47064, 32), 720}, // 768 SubgroupAvcSicConfigureSkcINTEL
  {IR(47213, 39), 724}, // 769 SubgroupAvcSicConvertToMcePayloadINTEL
  {IR(47753, 38), 736}, // 770 SubgroupAvcSicConvertToMceResultINTEL
  {IR(47528, 31), 731}, // 771 SubgroupAvcSicEvaluateIpeINTEL
  {IR(47606, 45), 733}, // 772 SubgroupAvcSicEvaluateWithDualReferenceINTEL
  {IR(47651, 46), 734}, // 773 SubgroupAvcSicEvaluateWithMultiReferenceINTEL
  {IR(47697, 56), 735}, // 774 SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL
  {IR(47559, 47), 732}, // 775 SubgroupAvcSicEvaluateWithSingleReferenceINTEL
  {IR(47870, 46), 739}, // 776 SubgroupAvcSicGetBestIpeChromaDistortionINTEL
  {IR(47826, 44), 738}, // 777 SubgroupAvcSicGetBestIpeLumaDistortionINTEL
  {IR(48091, 35), 744}, // 778 SubgroupAvcSicGetInterRawSadsINTEL
  {IR(47957, 36), 741}, // 779 SubgroupAvcSicGetIpeChromaModeINTEL
  {IR(47791, 35), 737}, // 780 SubgroupAvcSicGetIpeLumaShapeINTEL
  {IR(47174, 39), 723}, // 781 SubgroupAvcSicGetMotionVectorMaskINTEL
  {IR(47916, 41), 740}, // 782 SubgroupAvcSicGetPackedIpeLumaModesINTEL
  {IR(47993, 50), 742}, // 783 SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL
  {IR(48043, 48), 743}, // 784 SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL
  {IR(47034, 30), 719}, // 785 SubgroupAvcSicInitializeINTEL
  {IR(47394, 43), 728}, // 786 SubgroupAvcSicSetBilinearFilterEnableINTEL
  {IR(47485, 43), 730}, // 787 SubgroupAvcSicSetBlockBasedRawSkipSadINTEL
  {IR(47344, 50), 727}, // 788 SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL
  {IR(47296, 48), 726}, // 789 SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL
  {IR(47252, 44), 725}, // 790 SubgroupAvcSicSetIntraLumaShapePenaltyINTEL
  {IR(47437, 48), 729}, // 791 SubgroupAvcSicSetSkcForwardTransformEnableINTEL
  {IR(6721, 18), 364}, // 792 SubgroupBallotKHR
  {IR(51445, 27), 842}, // 793 SubgroupBlockPrefetchINTEL
  {IR(42344, 23), 596}, // 794 SubgroupBlockReadINTEL
  {IR(42367, 24), 597}, // 795 SubgroupBlockWriteINTEL
  {IR(36207, 27), 365}, // 796 SubgroupFirstInvocationKHR
  {IR(42391, 28), 598}, // 797 SubgroupImageBlockReadINTEL
  {IR(42419, 29), 599}, // 798 SubgroupImageBlockWriteINTEL
  {IR(42448, 33), 600}, // 799 SubgroupImageMediaBlockReadINTEL
  {IR(42481, 34), 601}, // 800 SubgroupImageMediaBlockWriteINTEL
  {IR(16363, 38), 848}, // 801 SubgroupMatrixMultiplyAccumulateINTEL
  {IR(36390, 26), 375}, // 802 SubgroupReadInvocationKHR
  {IR(42272, 25), 593}, // 803 SubgroupShuffleDownINTEL
  {IR(13961, 21), 592}, // 804 SubgroupShuffleINTEL
  {IR(42297, 23), 594}, // 805 SubgroupShuffleUpINTEL
  {IR(42320, 24), 595}, // 806 SubgroupShuffleXorINTEL
  {IR(33564, 7), 229}, // 807 Switch
  {IR(51259, 24), 838}, // 808 TaskSequenceAsyncALTERA
  {IR(51283, 23), 838}, // 809 TaskSequenceAsyncINTEL
  {IR(51210, 25), 837}, // 810 TaskSequenceCreateALTERA
  {IR(51235, 24), 837}, // 811 TaskSequenceCreateINTEL
  {IR(51306, 22), 839}, // 812 TaskSequenceGetALTERA
  {IR(51328, 21), 839}, // 813 TaskSequenceGetINTEL
  {IR(51349, 26), 840}, // 814 TaskSequenceReleaseALTERA
  {IR(51375, 25), 840}, // 815 TaskSequenceReleaseINTEL
  {IR(41615, 27), 569}, // 816 TensorLayoutSetBlockSizeNV
  {IR(41479, 28), 562}, // 817 TensorLayoutSetClampValueNV
  {IR(41408, 27), 559}, // 818 TensorLayoutSetDimensionNV
  {IR(41435, 24), 560}, // 819 TensorLayoutSetStrideNV
  {IR(41459, 20), 561}, // 820 TensorLayoutSliceNV
  {IR(35982, 19), 350}, // 821 TensorQuerySizeARM
  {IR(35953, 14), 348}, // 822 TensorReadARM
  {IR(41595, 20), 568}, // 823 TensorViewSetClipNV
  {IR(41526, 25), 564}, // 824 TensorViewSetDimensionNV
  {IR(41551, 22), 565}, // 825 TensorViewSetStrideNV
  {IR(35967, 15), 349}, // 826 TensorWriteARM
  {IR(36094, 20), 359}, // 827 TerminateInvocation
  {IR(36555, 16), 382}, // 828 TerminateRayKHR
  {IR(40449, 15), 529}, // 829 TerminateRayNV
  {IR(40472, 14), 531}, // 830 TraceMotionNV
  {IR(40464, 8), 530}, // 831 TraceNV
  {IR(36467, 12), 378}, // 832 TraceRayKHR
  {IR(40486, 17), 532}, // 833 TraceRayMotionNV
  {IR(31416, 10), 76}, // 834 Transpose
  {IR(40553, 29), 534}, // 835 TypeAccelerationStructureKHR
  {IR(40582, 28), 534}, // 836 TypeAccelerationStructureNV
  {IR(30365, 10), 25}, // 837 TypeArray
  {IR(43257, 37), 638}, // 838 TypeAvcImeDualReferenceStreaminINTEL
  {IR(42992, 23), 629}, // 839 TypeAvcImePayloadINTEL
  {IR(43174, 44), 636}, // 840 TypeAvcImeResultDualReferenceStreamoutINTEL
  {IR(43106, 22), 634}, // 841 TypeAvcImeResultINTEL
  {IR(43128, 46), 635}, // 842 TypeAvcImeResultSingleReferenceStreamoutINTEL
  {IR(43218, 39), 637}, // 843 TypeAvcImeSingleReferenceStreaminINTEL
  {IR(43061, 23), 632}, // 844 TypeAvcMcePayloadINTEL
  {IR(43084, 22), 633}, // 845 TypeAvcMceResultINTEL
  {IR(43015, 23), 630}, // 846 TypeAvcRefPayloadINTEL
  {IR(43294, 22), 639}, // 847 TypeAvcRefResultINTEL
  {IR(43038, 23), 631}, // 848 TypeAvcSicPayloadINTEL
  {IR(43316, 22), 640}, // 849 TypeAvcSicResultINTEL
  {IR(30127, 9), 17}, // 850 TypeBool
  {IR(37985, 14), 439}, // 851 TypeBufferEXT
  {IR(50925, 23), 827}, // 852 TypeBufferSurfaceINTEL
  {IR(36736, 25), 389}, // 853 TypeCooperativeMatrixKHR
  {IR(40875, 24), 544}, // 854 TypeCooperativeMatrixNV
  {IR(39038, 24), 483}, // 855 TypeCooperativeVectorNV
  {IR(30480, 16), 32}, // 856 TypeDeviceEvent
  {IR(30470, 10), 31}, // 857 TypeEvent
  {IR(30181, 10), 19}, // 858 TypeFloat
  {IR(30529, 19), 36}, // 859 TypeForwardPointer
  {IR(30457, 13), 30}, // 860 TypeFunction
  {IR(36081, 13), 357}, // 861 TypeGraphARM
  {IR(39810, 17), 506}, // 862 TypeHitObjectEXT
  {IR(38983, 16), 481}, // 863 TypeHitObjectNV
  {IR(30326, 10), 22}, // 864 TypeImage
  {IR(30136, 8), 18}, // 865 TypeInt
  {IR(30202, 11), 21}, // 866 TypeMatrix
  {IR(34911, 17), 300}, // 867 TypeNamedBarrier
  {IR(37787, 25), 431}, // 868 TypeNodePayloadArrayAMDX
  {IR(30403, 11), 28}, // 869 TypeOpaque
  {IR(30520, 9), 35}, // 870 TypePipe
  {IR(34789, 16), 295}, // 871 TypePipeStorage
  {IR(30445, 12), 29}, // 872 TypePointer
  {IR(30510, 10), 34}, // 873 TypeQueue
  {IR(37015, 16), 397}, // 874 TypeRayQueryKHR
  {IR(30496, 14), 33}, // 875 TypeReserveId
  {IR(30375, 17), 26}, // 876 TypeRuntimeArray
  {IR(30348, 17), 24}, // 877 TypeSampledImage
  {IR(30336, 12), 23}, // 878 TypeSampler
  {IR(30392, 11), 27}, // 879 TypeStruct
  {IR(50948, 25), 828}, // 880 TypeStructContinuedINTEL
  {IR(51400, 23), 841}, // 881 TypeTaskSequenceALTERA
  {IR(51423, 22), 841}, // 882 TypeTaskSequenceINTEL
  {IR(35897, 14), 347}, // 883 TypeTensorARM
  {IR(41351, 19), 556}, // 884 TypeTensorLayoutNV
  {IR(41370, 17), 557}, // 885 TypeTensorViewNV
  {IR(36114, 22), 360}, // 886 TypeUntypedPointerKHR
  {IR(30191, 11), 20}, // 887 TypeVector
  {IR(39022, 16), 483}, // 888 TypeVectorIdEXT
  {IR(42974, 18), 628}, // 889 TypeVmeImageINTEL
  {IR(30118, 9), 16}, // 890 TypeVoid
  {IR(42603, 13), 607}, // 891 UAddSatINTEL
  {IR(42630, 14), 609}, // 892 UAverageINTEL
  {IR(42665, 21), 611}, // 893 UAverageRoundedINTEL
  {IR(31933, 9), 103}, // 894 UConvert
  {IR(42515, 24), 602}, // 895 UCountLeadingZerosINTEL
  {IR(42539, 25), 603}, // 896 UCountTrailingZerosINTEL
  {IR(32145, 5), 123}, // 897 UDiv
  {IR(36631, 5), 384}, // 898 UDot
  {IR(36684, 11), 387}, // 899 UDotAccSat
  {IR(36695, 14), 387}, // 900 UDotAccSatKHR
  {IR(36636, 8), 384}, // 901 UDotKHR
  {IR(32505, 13), 160}, // 902 UGreaterThan
  {IR(32531, 18), 162}, // 903 UGreaterThanEqual
  {IR(32567, 10), 164}, // 904 ULessThan
  {IR(32587, 15), 166}, // 905 ULessThanEqual
  {IR(32160, 5), 126}, // 906 UMod
  {IR(42727, 15), 615}, // 907 UMul32x16INTEL
  {IR(32313, 13), 140}, // 908 UMulExtended
  {IR(42699, 13), 613}, // 909 USubSatINTEL
  {IR(29630, 6), 1}, // 910 Undef
  {IR(32410, 10), 151}, // 911 Unordered
  {IR(33595, 12), 233}, // 912 Unreachable
  {IR(36155, 22), 362}, // 913 UntypedAccessChainKHR
  {IR(36292, 22), 368}, // 914 UntypedArrayLengthKHR
  {IR(36442, 25), 377}, // 915 UntypedGroupAsyncCopyKHR
  {IR(38016, 28), 442}, // 916 UntypedImageTexelPointerEXT
  {IR(36177, 30), 363}, // 917 UntypedInBoundsAccessChainKHR
  {IR(36259, 33), 367}, // 918 UntypedInBoundsPtrAccessChainKHR
  {IR(36314, 19), 369}, // 919 UntypedPrefetchKHR
  {IR(36234, 25), 366}, // 920 UntypedPtrAccessChainKHR
  {IR(36136, 19), 361}, // 921 UntypedVariableKHR
  {IR(16429, 32), 850}, // 922 UntypedVariableLengthArrayINTEL
  {IR(30941, 9), 52}, // 923 Variable
  {IR(14417, 25), 745}, // 924 VariableLengthArrayINTEL
  {IR(31298, 21), 69}, // 925 VectorExtractDynamic
  {IR(31319, 20), 70}, // 926 VectorInsertDynamic
  {IR(31339, 14), 71}, // 927 VectorShuffle
  {IR(32221, 18), 133}, // 928 VectorTimesMatrix
  {IR(32185, 18), 131}, // 929 VectorTimesScalar
  {IR(42960, 14), 627}, // 930 VmeImageINTEL
  {IR(39352, 33), 492}, // 931 WritePackedPrimitiveIndices4x8NV
  {IR(33820, 10), 250}, // 932 WritePipe
  {IR(50213, 24), 807}, // 933 WritePipeBlockingALTERA
  {IR(50237, 23), 807}, // 934 WritePipeBlockingINTEL
}};

// Extended instruction descriptions, ordered by (extinst enum, opcode value).
// The fields in order are:
//   enum value
//   operands, an IndexRange into kOperandSpans
//   name, a character-counting IndexRange into kStrings
//   capabilities, an IndexRange into kCapabilitySpans
static const std::array<ExtInstDesc, 501> kExtInstByValue{{
    {0,IR(811, 2),IR(52309, 5),IR(1, 0)}, // CALL in arm_experimental_ml_operations
    {0,IR(765, 9),IR(52314, 8),IR(1, 0)}, // MIN_SAD in arm_motion_engine_100
    {1,IR(765, 9),IR(52322, 13),IR(1, 0)}, // MIN_SAD_COST in arm_motion_engine_100
    {2,IR(471, 8),IR(52335, 8),IR(1, 0)}, // RAW_SAD in arm_motion_engine_100
    {0,IR(0, 0),IR(52343, 14),IR(1, 0)}, // DebugInfoNone in debuginfo
    {1,IR(52, 3),IR(52357, 21),IR(1, 0)}, // DebugCompilationUnit in debuginfo
    {2,IR(813, 3),IR(52430, 15),IR(1, 0)}, // DebugTypeBasic in debuginfo
    {3,IR(816, 3),IR(52479, 17),IR(1, 0)}, // DebugTypePointer in debuginfo
    {4,IR(819, 2),IR(52534, 19),IR(1, 0)}, // DebugTypeQualifier in debuginfo
    {5,IR(168, 2),IR(52553, 15),IR(1, 0)}, // DebugTypeArray in debuginfo
    {6,IR(261, 2),IR(52568, 16),IR(1, 0)}, // DebugTypeVector in debuginfo
    {7,IR(821, 6),IR(52584, 13),IR(1, 0)}, // DebugTypedef in debuginfo
    {8,IR(168, 2),IR(52597, 18),IR(1, 0)}, // DebugTypeFunction in debuginfo
    {9,IR(827, 9),IR(52615, 14),IR(1, 0)}, // DebugTypeEnum in debuginfo
    {10,IR(836, 9),IR(52667, 19),IR(1, 0)}, // DebugTypeComposite in debuginfo
    {11,IR(845, 10),IR(52686, 16),IR(1, 0)}, // DebugTypeMember in debuginfo
    {12,IR(855, 5),IR(52702, 21),IR(1, 0)}, // DebugTypeInheritance in debuginfo
    {13,IR(36, 2),IR(52723, 21),IR(1, 0)}, // DebugTypePtrToMember in debuginfo
    {14,IR(168, 2),IR(52744, 18),IR(1, 0)}, // DebugTypeTemplate in debuginfo
    {15,IR(860, 6),IR(52762, 27),IR(1, 0)}, // DebugTypeTemplateParameter in debuginfo
    {16,IR(866, 5),IR(52789, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in debuginfo
    {17,IR(871, 5),IR(52824, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in debuginfo
    {18,IR(845, 10),IR(52855, 20),IR(1, 0)}, // DebugGlobalVariable in debuginfo
    {19,IR(876, 8),IR(52875, 25),IR(1, 0)}, // DebugFunctionDeclaration in debuginfo
    {20,IR(884, 11),IR(52900, 14),IR(1, 0)}, // DebugFunction in debuginfo
    {21,IR(895, 5),IR(52914, 18),IR(1, 0)}, // DebugLexicalBlock in debuginfo
    {22,IR(900, 3),IR(52932, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in debuginfo
    {23,IR(903, 2),IR(52963, 11),IR(1, 0)}, // DebugScope in debuginfo
    {24,IR(0, 0),IR(52974, 13),IR(1, 0)}, // DebugNoScope in debuginfo
    {25,IR(905, 3),IR(52987, 15),IR(1, 0)}, // DebugInlinedAt in debuginfo
    {26,IR(908, 7),IR(53044, 19),IR(1, 0)}, // DebugLocalVariable in debuginfo
    {27,IR(36, 2),IR(53063, 21),IR(1, 0)}, // DebugInlinedVariable in debuginfo
    {28,IR(33, 3),IR(53084, 13),IR(1, 0)}, // DebugDeclare in debuginfo
    {29,IR(375, 3),IR(53097, 11),IR(1, 0)}, // DebugValue in debuginfo
    {30,IR(915, 2),IR(53141, 15),IR(1, 0)}, // DebugOperation in debuginfo
    {31,IR(747, 1),IR(53156, 16),IR(1, 0)}, // DebugExpression in debuginfo
    {32,IR(917, 4),IR(53172, 14),IR(1, 0)}, // DebugMacroDef in debuginfo
    {33,IR(900, 3),IR(53186, 16),IR(1, 0)}, // DebugMacroUndef in debuginfo
    {1,IR(10, 1),IR(53202, 6),IR(1, 0)}, // Round in glsl_std_450
    {2,IR(10, 1),IR(53208, 10),IR(1, 0)}, // RoundEven in glsl_std_450
    {3,IR(10, 1),IR(53218, 6),IR(1, 0)}, // Trunc in glsl_std_450
    {4,IR(10, 1),IR(53224, 5),IR(1, 0)}, // FAbs in glsl_std_450
    {5,IR(10, 1),IR(53229, 5),IR(1, 0)}, // SAbs in glsl_std_450
    {6,IR(10, 1),IR(53234, 6),IR(1, 0)}, // FSign in glsl_std_450
    {7,IR(10, 1),IR(53240, 6),IR(1, 0)}, // SSign in glsl_std_450
    {8,IR(10, 1),IR(53246, 6),IR(1, 0)}, // Floor in glsl_std_450
    {9,IR(10, 1),IR(53252, 5),IR(1, 0)}, // Ceil in glsl_std_450
    {10,IR(10, 1),IR(53257, 6),IR(1, 0)}, // Fract in glsl_std_450
    {11,IR(10, 1),IR(53263, 8),IR(1, 0)}, // Radians in glsl_std_450
    {12,IR(10, 1),IR(53271, 8),IR(1, 0)}, // Degrees in glsl_std_450
    {13,IR(10, 1),IR(53279, 4),IR(1, 0)}, // Sin in glsl_std_450
    {14,IR(10, 1),IR(53283, 4),IR(1, 0)}, // Cos in glsl_std_450
    {15,IR(10, 1),IR(53287, 4),IR(1, 0)}, // Tan in glsl_std_450
    {16,IR(10, 1),IR(53291, 5),IR(1, 0)}, // Asin in glsl_std_450
    {17,IR(10, 1),IR(53296, 5),IR(1, 0)}, // Acos in glsl_std_450
    {18,IR(10, 1),IR(53301, 5),IR(1, 0)}, // Atan in glsl_std_450
    {19,IR(10, 1),IR(53306, 5),IR(1, 0)}, // Sinh in glsl_std_450
    {20,IR(10, 1),IR(53311, 5),IR(1, 0)}, // Cosh in glsl_std_450
    {21,IR(10, 1),IR(53316, 5),IR(1, 0)}, // Tanh in glsl_std_450
    {22,IR(10, 1),IR(53321, 6),IR(1, 0)}, // Asinh in glsl_std_450
    {23,IR(10, 1),IR(53327, 6),IR(1, 0)}, // Acosh in glsl_std_450
    {24,IR(10, 1),IR(53333, 6),IR(1, 0)}, // Atanh in glsl_std_450
    {25,IR(36, 2),IR(53339, 6),IR(1, 0)}, // Atan2 in glsl_std_450
    {26,IR(36, 2),IR(53345, 4),IR(1, 0)}, // Pow in glsl_std_450
    {27,IR(10, 1),IR(53349, 4),IR(1, 0)}, // Exp in glsl_std_450
    {28,IR(10, 1),IR(53353, 4),IR(1, 0)}, // Log in glsl_std_450
    {29,IR(10, 1),IR(53357, 5),IR(1, 0)}, // Exp2 in glsl_std_450
    {30,IR(10, 1),IR(53362, 5),IR(1, 0)}, // Log2 in glsl_std_450
    {31,IR(10, 1),IR(53367, 5),IR(1, 0)}, // Sqrt in glsl_std_450
    {32,IR(10, 1),IR(53372, 12),IR(1, 0)}, // InverseSqrt in glsl_std_450
    {33,IR(10, 1),IR(53384, 12),IR(1, 0)}, // Determinant in glsl_std_450
    {34,IR(10, 1),IR(53396, 14),IR(1, 0)}, // MatrixInverse in glsl_std_450
    {35,IR(36, 2),IR(53410, 5),IR(1, 0)}, // Modf in glsl_std_450
    {36,IR(10, 1),IR(53415, 11),IR(1, 0)}, // ModfStruct in glsl_std_450
    {37,IR(36, 2),IR(53426, 5),IR(1, 0)}, // FMin in glsl_std_450
    {38,IR(36, 2),IR(53431, 5),IR(1, 0)}, // UMin in glsl_std_450
    {39,IR(36, 2),IR(53436, 5),IR(1, 0)}, // SMin in glsl_std_450
    {40,IR(36, 2),IR(53441, 5),IR(1, 0)}, // FMax in glsl_std_450
    {41,IR(36, 2),IR(53446, 5),IR(1, 0)}, // UMax in glsl_std_450
    {42,IR(36, 2),IR(53451, 5),IR(1, 0)}, // SMax in glsl_std_450
    {43,IR(33, 3),IR(53456, 7),IR(1, 0)}, // FClamp in glsl_std_450
    {44,IR(33, 3),IR(53463, 7),IR(1, 0)}, // UClamp in glsl_std_450
    {45,IR(33, 3),IR(53470, 7),IR(1, 0)}, // SClamp in glsl_std_450
    {46,IR(33, 3),IR(53477, 5),IR(1, 0)}, // FMix in glsl_std_450
    {47,IR(33, 3),IR(53482, 5),IR(1, 0)}, // IMix in glsl_std_450
    {48,IR(36, 2),IR(53487, 5),IR(1, 0)}, // Step in glsl_std_450
    {49,IR(33, 3),IR(53492, 11),IR(1, 0)}, // SmoothStep in glsl_std_450
    {50,IR(33, 3),IR(53503, 4),IR(1, 0)}, // Fma in glsl_std_450
    {51,IR(36, 2),IR(53507, 6),IR(1, 0)}, // Frexp in glsl_std_450
    {52,IR(10, 1),IR(53513, 12),IR(1, 0)}, // FrexpStruct in glsl_std_450
    {53,IR(36, 2),IR(53525, 6),IR(1, 0)}, // Ldexp in glsl_std_450
    {54,IR(10, 1),IR(53531, 13),IR(1, 0)}, // PackSnorm4x8 in glsl_std_450
    {55,IR(10, 1),IR(53544, 13),IR(1, 0)}, // PackUnorm4x8 in glsl_std_450
    {56,IR(10, 1),IR(53557, 14),IR(1, 0)}, // PackSnorm2x16 in glsl_std_450
    {57,IR(10, 1),IR(53571, 14),IR(1, 0)}, // PackUnorm2x16 in glsl_std_450
    {58,IR(10, 1),IR(53585, 13),IR(1, 0)}, // PackHalf2x16 in glsl_std_450
    {59,IR(10, 1),IR(53598, 15),IR(347, 1)}, // PackDouble2x32 in glsl_std_450
    {60,IR(10, 1),IR(53613, 16),IR(1, 0)}, // UnpackSnorm2x16 in glsl_std_450
    {61,IR(10, 1),IR(53629, 16),IR(1, 0)}, // UnpackUnorm2x16 in glsl_std_450
    {62,IR(10, 1),IR(53645, 15),IR(1, 0)}, // UnpackHalf2x16 in glsl_std_450
    {63,IR(10, 1),IR(53660, 15),IR(1, 0)}, // UnpackSnorm4x8 in glsl_std_450
    {64,IR(10, 1),IR(53675, 15),IR(1, 0)}, // UnpackUnorm4x8 in glsl_std_450
    {65,IR(10, 1),IR(53690, 17),IR(347, 1)}, // UnpackDouble2x32 in glsl_std_450
    {66,IR(10, 1),IR(53707, 7),IR(1, 0)}, // Length in glsl_std_450
    {67,IR(36, 2),IR(53714, 9),IR(1, 0)}, // Distance in glsl_std_450
    {68,IR(36, 2),IR(53723, 6),IR(1, 0)}, // Cross in glsl_std_450
    {69,IR(10, 1),IR(53729, 10),IR(1, 0)}, // Normalize in glsl_std_450
    {70,IR(33, 3),IR(53739, 12),IR(1, 0)}, // FaceForward in glsl_std_450
    {71,IR(36, 2),IR(53751, 8),IR(1, 0)}, // Reflect in glsl_std_450
    {72,IR(33, 3),IR(53759, 8),IR(1, 0)}, // Refract in glsl_std_450
    {73,IR(10, 1),IR(53767, 9),IR(1, 0)}, // FindILsb in glsl_std_450
    {74,IR(10, 1),IR(53776, 9),IR(1, 0)}, // FindSMsb in glsl_std_450
    {75,IR(10, 1),IR(53785, 9),IR(1, 0)}, // FindUMsb in glsl_std_450
    {76,IR(10, 1),IR(53794, 22),IR(348, 1)}, // InterpolateAtCentroid in glsl_std_450
    {77,IR(36, 2),IR(53816, 20),IR(348, 1)}, // InterpolateAtSample in glsl_std_450
    {78,IR(36, 2),IR(53836, 20),IR(348, 1)}, // InterpolateAtOffset in glsl_std_450
    {79,IR(36, 2),IR(53856, 5),IR(1, 0)}, // NMin in glsl_std_450
    {80,IR(36, 2),IR(53861, 5),IR(1, 0)}, // NMax in glsl_std_450
    {81,IR(33, 3),IR(53866, 7),IR(1, 0)}, // NClamp in glsl_std_450
    {1,IR(399, 5),IR(5940, 7),IR(1, 0)}, // Kernel in nonsemantic_clspvreflection
    {2,IR(921, 5),IR(53873, 13),IR(1, 0)}, // ArgumentInfo in nonsemantic_clspvreflection
    {3,IR(593, 5),IR(53886, 22),IR(1, 0)}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {4,IR(593, 5),IR(53908, 16),IR(1, 0)}, // ArgumentUniform in nonsemantic_clspvreflection
    {5,IR(557, 7),IR(53924, 25),IR(1, 0)}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {6,IR(557, 7),IR(53949, 19),IR(1, 0)}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {7,IR(593, 5),IR(53968, 24),IR(1, 0)}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {8,IR(593, 5),IR(53992, 21),IR(1, 0)}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {9,IR(593, 5),IR(54013, 21),IR(1, 0)}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {10,IR(593, 5),IR(54034, 16),IR(1, 0)}, // ArgumentSampler in nonsemantic_clspvreflection
    {11,IR(593, 5),IR(54050, 18),IR(1, 0)}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {12,IR(33, 3),IR(54068, 26),IR(1, 0)}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {13,IR(33, 3),IR(54094, 25),IR(1, 0)}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {14,IR(10, 1),IR(54119, 20),IR(1, 0)}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {15,IR(36, 2),IR(54139, 25),IR(1, 0)}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {16,IR(36, 2),IR(54164, 30),IR(1, 0)}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {17,IR(36, 2),IR(54194, 23),IR(1, 0)}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {18,IR(36, 2),IR(54217, 25),IR(1, 0)}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {19,IR(36, 2),IR(54242, 26),IR(1, 0)}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {20,IR(36, 2),IR(54268, 30),IR(1, 0)}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {21,IR(33, 3),IR(54298, 26),IR(1, 0)}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {22,IR(33, 3),IR(54324, 20),IR(1, 0)}, // ConstantDataUniform in nonsemantic_clspvreflection
    {23,IR(33, 3),IR(9457, 15),IR(1, 0)}, // LiteralSampler in nonsemantic_clspvreflection
    {24,IR(296, 4),IR(54344, 30),IR(1, 0)}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {25,IR(10, 1),IR(54374, 28),IR(1, 0)}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {26,IR(593, 5),IR(54402, 28),IR(1, 0)}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {27,IR(557, 7),IR(54430, 23),IR(1, 0)}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {28,IR(33, 3),IR(54453, 35),IR(1, 0)}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {29,IR(33, 3),IR(54488, 38),IR(1, 0)}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {30,IR(296, 4),IR(54526, 42),IR(1, 0)}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {31,IR(296, 4),IR(54568, 45),IR(1, 0)}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {32,IR(535, 6),IR(54613, 37),IR(1, 0)}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {33,IR(535, 6),IR(54650, 40),IR(1, 0)}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {34,IR(593, 5),IR(54690, 27),IR(1, 0)}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {35,IR(593, 5),IR(54717, 27),IR(1, 0)}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {36,IR(33, 3),IR(54744, 32),IR(1, 0)}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {37,IR(33, 3),IR(54776, 40),IR(1, 0)}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {38,IR(375, 3),IR(54816, 11),IR(1, 0)}, // PrintfInfo in nonsemantic_clspvreflection
    {39,IR(33, 3),IR(54827, 26),IR(1, 0)}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {40,IR(33, 3),IR(54853, 32),IR(1, 0)}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {41,IR(296, 4),IR(54885, 34),IR(1, 0)}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {42,IR(36, 2),IR(54919, 22),IR(1, 0)}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {1,IR(168, 2),IR(54941, 12),IR(1, 0)}, // DebugPrintf in nonsemantic_debugprintf
    {1,IR(36, 2),IR(54953, 11),IR(1, 0)}, // DebugGraph in nonsemantic_graph_debuginfo
    {2,IR(375, 3),IR(53141, 15),IR(1, 0)}, // DebugOperation in nonsemantic_graph_debuginfo
    {3,IR(926, 3),IR(54964, 12),IR(1, 0)}, // DebugTensor in nonsemantic_graph_debuginfo
    {0,IR(0, 0),IR(52343, 14),IR(1, 0)}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {1,IR(296, 4),IR(52357, 21),IR(1, 0)}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {2,IR(593, 5),IR(52430, 15),IR(1, 0)}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {3,IR(33, 3),IR(52479, 17),IR(1, 0)}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {4,IR(36, 2),IR(52534, 19),IR(1, 0)}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {5,IR(168, 2),IR(52553, 15),IR(1, 0)}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {6,IR(36, 2),IR(52568, 16),IR(1, 0)}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {7,IR(535, 6),IR(52584, 13),IR(1, 0)}, // DebugTypedef in nonsemantic_shader_debuginfo
    {8,IR(375, 3),IR(52597, 18),IR(1, 0)}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {9,IR(929, 9),IR(52615, 14),IR(1, 0)}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {10,IR(938, 10),IR(52667, 19),IR(1, 0)}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {11,IR(948, 9),IR(52686, 16),IR(1, 0)}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {12,IR(296, 4),IR(52702, 21),IR(1, 0)}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {13,IR(36, 2),IR(52723, 21),IR(1, 0)}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {14,IR(168, 2),IR(52744, 18),IR(1, 0)}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {15,IR(535, 6),IR(52762, 27),IR(1, 0)}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {16,IR(675, 5),IR(52789, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {17,IR(957, 5),IR(52824, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {18,IR(962, 10),IR(52855, 20),IR(1, 0)}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {19,IR(471, 8),IR(52875, 25),IR(1, 0)}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {20,IR(962, 10),IR(52900, 14),IR(1, 0)}, // DebugFunction in nonsemantic_shader_debuginfo
    {21,IR(593, 5),IR(52914, 18),IR(1, 0)}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {22,IR(33, 3),IR(52932, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {23,IR(903, 2),IR(52963, 11),IR(1, 0)}, // DebugScope in nonsemantic_shader_debuginfo
    {24,IR(0, 0),IR(52974, 13),IR(1, 0)}, // DebugNoScope in nonsemantic_shader_debuginfo
    {25,IR(926, 3),IR(52987, 15),IR(1, 0)}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {26,IR(972, 8),IR(53044, 19),IR(1, 0)}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {27,IR(36, 2),IR(53063, 21),IR(1, 0)}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {28,IR(980, 4),IR(53084, 13),IR(1, 0)}, // DebugDeclare in nonsemantic_shader_debuginfo
    {29,IR(980, 4),IR(53097, 11),IR(1, 0)}, // DebugValue in nonsemantic_shader_debuginfo
    {30,IR(168, 2),IR(53141, 15),IR(1, 0)}, // DebugOperation in nonsemantic_shader_debuginfo
    {31,IR(747, 1),IR(53156, 16),IR(1, 0)}, // DebugExpression in nonsemantic_shader_debuginfo
    {32,IR(580, 4),IR(53172, 14),IR(1, 0)}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {33,IR(33, 3),IR(53186, 16),IR(1, 0)}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {34,IR(516, 7),IR(54976, 20),IR(1, 0)}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {35,IR(903, 2),IR(54996, 12),IR(1, 0)}, // DebugSource in nonsemantic_shader_debuginfo
    {101,IR(36, 2),IR(55008, 24),IR(1, 0)}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {102,IR(10, 1),IR(55032, 21),IR(1, 0)}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {103,IR(675, 5),IR(55053, 10),IR(1, 0)}, // DebugLine in nonsemantic_shader_debuginfo
    {104,IR(0, 0),IR(55063, 12),IR(1, 0)}, // DebugNoLine in nonsemantic_shader_debuginfo
    {105,IR(36, 2),IR(55075, 21),IR(1, 0)}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {106,IR(10, 1),IR(55096, 17),IR(1, 0)}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {107,IR(296, 4),IR(55113, 16),IR(1, 0)}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {108,IR(33, 3),IR(55129, 16),IR(1, 0)}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {109,IR(36, 2),IR(55145, 21),IR(1, 0)}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {110,IR(675, 5),IR(55166, 30),IR(1, 0)}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {1,IR(765, 9),IR(55196, 14),IR(1, 0)}, // Configuration in nonsemantic_vkspreflection
    {2,IR(10, 1),IR(55210, 13),IR(1, 0)}, // StartCounter in nonsemantic_vkspreflection
    {3,IR(10, 1),IR(55223, 12),IR(1, 0)}, // StopCounter in nonsemantic_vkspreflection
    {4,IR(296, 4),IR(55235, 14),IR(1, 0)}, // PushConstants in nonsemantic_vkspreflection
    {5,IR(33, 3),IR(55249, 23),IR(1, 0)}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {6,IR(984, 15),IR(55272, 20),IR(1, 0)}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {7,IR(999, 33),IR(55292, 19),IR(1, 0)}, // DescriptorSetImage in nonsemantic_vkspreflection
    {8,IR(1032, 19),IR(55311, 21),IR(1, 0)}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {0,IR(0, 0),IR(52343, 14),IR(1, 0)}, // DebugInfoNone in opencl_debuginfo_100
    {1,IR(1051, 4),IR(52357, 21),IR(1, 0)}, // DebugCompilationUnit in opencl_debuginfo_100
    {2,IR(1055, 3),IR(52430, 15),IR(1, 0)}, // DebugTypeBasic in opencl_debuginfo_100
    {3,IR(1058, 3),IR(52479, 17),IR(1, 0)}, // DebugTypePointer in opencl_debuginfo_100
    {4,IR(1061, 2),IR(52534, 19),IR(1, 0)}, // DebugTypeQualifier in opencl_debuginfo_100
    {5,IR(168, 2),IR(52553, 15),IR(1, 0)}, // DebugTypeArray in opencl_debuginfo_100
    {6,IR(261, 2),IR(52568, 16),IR(1, 0)}, // DebugTypeVector in opencl_debuginfo_100
    {7,IR(821, 6),IR(52584, 13),IR(1, 0)}, // DebugTypedef in opencl_debuginfo_100
    {8,IR(1063, 3),IR(52597, 18),IR(1, 0)}, // DebugTypeFunction in opencl_debuginfo_100
    {9,IR(1066, 9),IR(52615, 14),IR(1, 0)}, // DebugTypeEnum in opencl_debuginfo_100
    {10,IR(1075, 10),IR(52667, 19),IR(1, 0)}, // DebugTypeComposite in opencl_debuginfo_100
    {11,IR(1085, 10),IR(52686, 16),IR(1, 0)}, // DebugTypeMember in opencl_debuginfo_100
    {12,IR(1095, 5),IR(52702, 21),IR(1, 0)}, // DebugTypeInheritance in opencl_debuginfo_100
    {13,IR(36, 2),IR(52723, 21),IR(1, 0)}, // DebugTypePtrToMember in opencl_debuginfo_100
    {14,IR(168, 2),IR(52744, 18),IR(1, 0)}, // DebugTypeTemplate in opencl_debuginfo_100
    {15,IR(860, 6),IR(52762, 27),IR(1, 0)}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {16,IR(866, 5),IR(52789, 35),IR(1, 0)}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {17,IR(871, 5),IR(52824, 31),IR(1, 0)}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {18,IR(1085, 10),IR(52855, 20),IR(1, 0)}, // DebugGlobalVariable in opencl_debuginfo_100
    {19,IR(1100, 8),IR(52875, 25),IR(1, 0)}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {20,IR(1108, 11),IR(52900, 14),IR(1, 0)}, // DebugFunction in opencl_debuginfo_100
    {21,IR(895, 5),IR(52914, 18),IR(1, 0)}, // DebugLexicalBlock in opencl_debuginfo_100
    {22,IR(900, 3),IR(52932, 31),IR(1, 0)}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {23,IR(903, 2),IR(52963, 11),IR(1, 0)}, // DebugScope in opencl_debuginfo_100
    {24,IR(0, 0),IR(52974, 13),IR(1, 0)}, // DebugNoScope in opencl_debuginfo_100
    {25,IR(905, 3),IR(52987, 15),IR(1, 0)}, // DebugInlinedAt in opencl_debuginfo_100
    {26,IR(1119, 8),IR(53044, 19),IR(1, 0)}, // DebugLocalVariable in opencl_debuginfo_100
    {27,IR(36, 2),IR(53063, 21),IR(1, 0)}, // DebugInlinedVariable in opencl_debuginfo_100
    {28,IR(33, 3),IR(53084, 13),IR(1, 0)}, // DebugDeclare in opencl_debuginfo_100
    {29,IR(980, 4),IR(53097, 11),IR(1, 0)}, // DebugValue in opencl_debuginfo_100
    {30,IR(1127, 2),IR(53141, 15),IR(1, 0)}, // DebugOperation in opencl_debuginfo_100
    {31,IR(747, 1),IR(53156, 16),IR(1, 0)}, // DebugExpression in opencl_debuginfo_100
    {32,IR(917, 4),IR(53172, 14),IR(1, 0)}, // DebugMacroDef in opencl_debuginfo_100
    {33,IR(900, 3),IR(53186, 16),IR(1, 0)}, // DebugMacroUndef in opencl_debuginfo_100
    {34,IR(1129, 7),IR(54976, 20),IR(1, 0)}, // DebugImportedEntity in opencl_debuginfo_100
    {35,IR(903, 2),IR(54996, 12),IR(1, 0)}, // DebugSource in opencl_debuginfo_100
    {36,IR(1136, 8),IR(55632, 17),IR(1, 0)}, // DebugModuleINTEL in opencl_debuginfo_100
    {0,IR(10, 1),IR(55649, 5),IR(1, 0)}, // acos in opencl_std_100
    {1,IR(10, 1),IR(55654, 6),IR(1, 0)}, // acosh in opencl_std_100
    {2,IR(10, 1),IR(55660, 7),IR(1, 0)}, // acospi in opencl_std_100
    {3,IR(10, 1),IR(55667, 5),IR(1, 0)}, // asin in opencl_std_100
    {4,IR(10, 1),IR(55672, 6),IR(1, 0)}, // asinh in opencl_std_100
    {5,IR(10, 1),IR(55678, 7),IR(1, 0)}, // asinpi in opencl_std_100
    {6,IR(10, 1),IR(55685, 5),IR(1, 0)}, // atan in opencl_std_100
    {7,IR(36, 2),IR(55690, 6),IR(1, 0)}, // atan2 in opencl_std_100
    {8,IR(10, 1),IR(55696, 6),IR(1, 0)}, // atanh in opencl_std_100
    {9,IR(10, 1),IR(55702, 7),IR(1, 0)}, // atanpi in opencl_std_100
    {10,IR(36, 2),IR(55709, 8),IR(1, 0)}, // atan2pi in opencl_std_100
    {11,IR(10, 1),IR(55717, 5),IR(1, 0)}, // cbrt in opencl_std_100
    {12,IR(10, 1),IR(55722, 5),IR(1, 0)}, // ceil in opencl_std_100
    {13,IR(36, 2),IR(55727, 9),IR(1, 0)}, // copysign in opencl_std_100
    {14,IR(10, 1),IR(55736, 4),IR(1, 0)}, // cos in opencl_std_100
    {15,IR(10, 1),IR(55740, 5),IR(1, 0)}, // cosh in opencl_std_100
    {16,IR(10, 1),IR(55745, 6),IR(1, 0)}, // cospi in opencl_std_100
    {17,IR(10, 1),IR(55751, 5),IR(1, 0)}, // erfc in opencl_std_100
    {18,IR(10, 1),IR(55756, 4),IR(1, 0)}, // erf in opencl_std_100
    {19,IR(10, 1),IR(55760, 4),IR(1, 0)}, // exp in opencl_std_100
    {20,IR(10, 1),IR(55764, 5),IR(1, 0)}, // exp2 in opencl_std_100
    {21,IR(10, 1),IR(55769, 6),IR(1, 0)}, // exp10 in opencl_std_100
    {22,IR(10, 1),IR(55775, 6),IR(1, 0)}, // expm1 in opencl_std_100
    {23,IR(10, 1),IR(55781, 5),IR(1, 0)}, // fabs in opencl_std_100
    {24,IR(36, 2),IR(55786, 5),IR(1, 0)}, // fdim in opencl_std_100
    {25,IR(10, 1),IR(55791, 6),IR(1, 0)}, // floor in opencl_std_100
    {26,IR(33, 3),IR(55797, 4),IR(1, 0)}, // fma in opencl_std_100
    {27,IR(36, 2),IR(55801, 5),IR(1, 0)}, // fmax in opencl_std_100
    {28,IR(36, 2),IR(55806, 5),IR(1, 0)}, // fmin in opencl_std_100
    {29,IR(36, 2),IR(55811, 5),IR(1, 0)}, // fmod in opencl_std_100
    {30,IR(36, 2),IR(55816, 6),IR(1, 0)}, // fract in opencl_std_100
    {31,IR(36, 2),IR(55822, 6),IR(1, 0)}, // frexp in opencl_std_100
    {32,IR(36, 2),IR(55828, 6),IR(1, 0)}, // hypot in opencl_std_100
    {33,IR(10, 1),IR(55834, 6),IR(1, 0)}, // ilogb in opencl_std_100
    {34,IR(36, 2),IR(55840, 6),IR(1, 0)}, // ldexp in opencl_std_100
    {35,IR(10, 1),IR(55846, 7),IR(1, 0)}, // lgamma in opencl_std_100
    {36,IR(36, 2),IR(55853, 9),IR(1, 0)}, // lgamma_r in opencl_std_100
    {37,IR(10, 1),IR(55862, 4),IR(1, 0)}, // log in opencl_std_100
    {38,IR(10, 1),IR(55866, 5),IR(1, 0)}, // log2 in opencl_std_100
    {39,IR(10, 1),IR(55871, 6),IR(1, 0)}, // log10 in opencl_std_100
    {40,IR(10, 1),IR(55877, 6),IR(1, 0)}, // log1p in opencl_std_100
    {41,IR(10, 1),IR(55883, 5),IR(1, 0)}, // logb in opencl_std_100
    {42,IR(33, 3),IR(55888, 4),IR(1, 0)}, // mad in opencl_std_100
    {43,IR(36, 2),IR(55892, 7),IR(1, 0)}, // maxmag in opencl_std_100
    {44,IR(36, 2),IR(55899, 7),IR(1, 0)}, // minmag in opencl_std_100
    {45,IR(36, 2),IR(55906, 5),IR(1, 0)}, // modf in opencl_std_100
    {46,IR(10, 1),IR(55911, 4),IR(1, 0)}, // nan in opencl_std_100
    {47,IR(36, 2),IR(55915, 10),IR(1, 0)}, // nextafter in opencl_std_100
    {48,IR(36, 2),IR(55925, 4),IR(1, 0)}, // pow in opencl_std_100
    {49,IR(36, 2),IR(55929, 5),IR(1, 0)}, // pown in opencl_std_100
    {50,IR(36, 2),IR(55934, 5),IR(1, 0)}, // powr in opencl_std_100
    {51,IR(36, 2),IR(55939, 10),IR(1, 0)}, // remainder in opencl_std_100
    {52,IR(33, 3),IR(55949, 7),IR(1, 0)}, // remquo in opencl_std_100
    {53,IR(10, 1),IR(55956, 5),IR(1, 0)}, // rint in opencl_std_100
    {54,IR(36, 2),IR(55961, 6),IR(1, 0)}, // rootn in opencl_std_100
    {55,IR(10, 1),IR(55967, 6),IR(1, 0)}, // round in opencl_std_100
    {56,IR(10, 1),IR(55973, 6),IR(1, 0)}, // rsqrt in opencl_std_100
    {57,IR(10, 1),IR(55979, 4),IR(1, 0)}, // sin in opencl_std_100
    {58,IR(36, 2),IR(55983, 7),IR(1, 0)}, // sincos in opencl_std_100
    {59,IR(10, 1),IR(55990, 5),IR(1, 0)}, // sinh in opencl_std_100
    {60,IR(10, 1),IR(55995, 6),IR(1, 0)}, // sinpi in opencl_std_100
    {61,IR(10, 1),IR(56001, 5),IR(1, 0)}, // sqrt in opencl_std_100
    {62,IR(10, 1),IR(56006, 4),IR(1, 0)}, // tan in opencl_std_100
    {63,IR(10, 1),IR(56010, 5),IR(1, 0)}, // tanh in opencl_std_100
    {64,IR(10, 1),IR(56015, 6),IR(1, 0)}, // tanpi in opencl_std_100
    {65,IR(10, 1),IR(56021, 7),IR(1, 0)}, // tgamma in opencl_std_100
    {66,IR(10, 1),IR(56028, 6),IR(1, 0)}, // trunc in opencl_std_100
    {67,IR(10, 1),IR(56034, 9),IR(1, 0)}, // half_cos in opencl_std_100
    {68,IR(36, 2),IR(56043, 12),IR(1, 0)}, // half_divide in opencl_std_100
    {69,IR(10, 1),IR(56055, 9),IR(1, 0)}, // half_exp in opencl_std_100
    {70,IR(10, 1),IR(56064, 10),IR(1, 0)}, // half_exp2 in opencl_std_100
    {71,IR(10, 1),IR(56074, 11),IR(1, 0)}, // half_exp10 in opencl_std_100
    {72,IR(10, 1),IR(56085, 9),IR(1, 0)}, // half_log in opencl_std_100
    {73,IR(10, 1),IR(56094, 10),IR(1, 0)}, // half_log2 in opencl_std_100
    {74,IR(10, 1),IR(56104, 11),IR(1, 0)}, // half_log10 in opencl_std_100
    {75,IR(36, 2),IR(56115, 10),IR(1, 0)}, // half_powr in opencl_std_100
    {76,IR(10, 1),IR(56125, 11),IR(1, 0)}, // half_recip in opencl_std_100
    {77,IR(10, 1),IR(56136, 11),IR(1, 0)}, // half_rsqrt in opencl_std_100
    {78,IR(10, 1),IR(56147, 9),IR(1, 0)}, // half_sin in opencl_std_100
    {79,IR(10, 1),IR(56156, 10),IR(1, 0)}, // half_sqrt in opencl_std_100
    {80,IR(10, 1),IR(56166, 9),IR(1, 0)}, // half_tan in opencl_std_100
    {81,IR(10, 1),IR(56175, 11),IR(1, 0)}, // native_cos in opencl_std_100
    {82,IR(36, 2),IR(56186, 14),IR(1, 0)}, // native_divide in opencl_std_100
    {83,IR(10, 1),IR(56200, 11),IR(1, 0)}, // native_exp in opencl_std_100
    {84,IR(10, 1),IR(56211, 12),IR(1, 0)}, // native_exp2 in opencl_std_100
    {85,IR(10, 1),IR(56223, 13),IR(1, 0)}, // native_exp10 in opencl_std_100
    {86,IR(10, 1),IR(56236, 11),IR(1, 0)}, // native_log in opencl_std_100
    {87,IR(10, 1),IR(56247, 12),IR(1, 0)}, // native_log2 in opencl_std_100
    {88,IR(10, 1),IR(56259, 13),IR(1, 0)}, // native_log10 in opencl_std_100
    {89,IR(36, 2),IR(56272, 12),IR(1, 0)}, // native_powr in opencl_std_100
    {90,IR(10, 1),IR(56284, 13),IR(1, 0)}, // native_recip in opencl_std_100
    {91,IR(10, 1),IR(56297, 13),IR(1, 0)}, // native_rsqrt in opencl_std_100
    {92,IR(10, 1),IR(56310, 11),IR(1, 0)}, // native_sin in opencl_std_100
    {93,IR(10, 1),IR(56321, 12),IR(1, 0)}, // native_sqrt in opencl_std_100
    {94,IR(10, 1),IR(56333, 11),IR(1, 0)}, // native_tan in opencl_std_100
    {95,IR(33, 3),IR(56344, 7),IR(1, 0)}, // fclamp in opencl_std_100
    {96,IR(10, 1),IR(56351, 8),IR(1, 0)}, // degrees in opencl_std_100
    {97,IR(36, 2),IR(56359, 12),IR(1, 0)}, // fmax_common in opencl_std_100
    {98,IR(36, 2),IR(56371, 12),IR(1, 0)}, // fmin_common in opencl_std_100
    {99,IR(33, 3),IR(56383, 4),IR(1, 0)}, // mix in opencl_std_100
    {100,IR(10, 1),IR(56387, 8),IR(1, 0)}, // radians in opencl_std_100
    {101,IR(36, 2),IR(56395, 5),IR(1, 0)}, // step in opencl_std_100
    {102,IR(33, 3),IR(56400, 11),IR(1, 0)}, // smoothstep in opencl_std_100
    {103,IR(10, 1),IR(56411, 5),IR(1, 0)}, // sign in opencl_std_100
    {104,IR(36, 2),IR(56416, 6),IR(1, 0)}, // cross in opencl_std_100
    {105,IR(36, 2),IR(56422, 9),IR(1, 0)}, // distance in opencl_std_100
    {106,IR(10, 1),IR(56431, 7),IR(1, 0)}, // length in opencl_std_100
    {107,IR(10, 1),IR(56438, 10),IR(1, 0)}, // normalize in opencl_std_100
    {108,IR(36, 2),IR(56448, 14),IR(1, 0)}, // fast_distance in opencl_std_100
    {109,IR(10, 1),IR(56462, 12),IR(1, 0)}, // fast_length in opencl_std_100
    {110,IR(10, 1),IR(56474, 15),IR(1, 0)}, // fast_normalize in opencl_std_100
    {141,IR(10, 1),IR(56489, 6),IR(1, 0)}, // s_abs in opencl_std_100
    {142,IR(36, 2),IR(56495, 11),IR(1, 0)}, // s_abs_diff in opencl_std_100
    {143,IR(36, 2),IR(56506, 10),IR(1, 0)}, // s_add_sat in opencl_std_100
    {144,IR(36, 2),IR(56516, 10),IR(1, 0)}, // u_add_sat in opencl_std_100
    {145,IR(36, 2),IR(56526, 7),IR(1, 0)}, // s_hadd in opencl_std_100
    {146,IR(36, 2),IR(56533, 7),IR(1, 0)}, // u_hadd in opencl_std_100
    {147,IR(36, 2),IR(56540, 8),IR(1, 0)}, // s_rhadd in opencl_std_100
    {148,IR(36, 2),IR(56548, 8),IR(1, 0)}, // u_rhadd in opencl_std_100
    {149,IR(33, 3),IR(56556, 8),IR(1, 0)}, // s_clamp in opencl_std_100
    {150,IR(33, 3),IR(56564, 8),IR(1, 0)}, // u_clamp in opencl_std_100
    {151,IR(10, 1),IR(56572, 4),IR(1, 0)}, // clz in opencl_std_100
    {152,IR(10, 1),IR(56576, 4),IR(1, 0)}, // ctz in opencl_std_100
    {153,IR(33, 3),IR(56580, 9),IR(1, 0)}, // s_mad_hi in opencl_std_100
    {154,IR(33, 3),IR(56589, 10),IR(1, 0)}, // u_mad_sat in opencl_std_100
    {155,IR(33, 3),IR(56599, 10),IR(1, 0)}, // s_mad_sat in opencl_std_100
    {156,IR(36, 2),IR(56609, 6),IR(1, 0)}, // s_max in opencl_std_100
    {157,IR(36, 2),IR(56615, 6),IR(1, 0)}, // u_max in opencl_std_100
    {158,IR(36, 2),IR(56621, 6),IR(1, 0)}, // s_min in opencl_std_100
    {159,IR(36, 2),IR(56627, 6),IR(1, 0)}, // u_min in opencl_std_100
    {160,IR(36, 2),IR(56633, 9),IR(1, 0)}, // s_mul_hi in opencl_std_100
    {161,IR(36, 2),IR(56642, 7),IR(1, 0)}, // rotate in opencl_std_100
    {162,IR(36, 2),IR(56649, 10),IR(1, 0)}, // s_sub_sat in opencl_std_100
    {163,IR(36, 2),IR(56659, 10),IR(1, 0)}, // u_sub_sat in opencl_std_100
    {164,IR(36, 2),IR(56669, 11),IR(1, 0)}, // u_upsample in opencl_std_100
    {165,IR(36, 2),IR(56680, 11),IR(1, 0)}, // s_upsample in opencl_std_100
    {166,IR(10, 1),IR(56691, 9),IR(1, 0)}, // popcount in opencl_std_100
    {167,IR(33, 3),IR(56700, 8),IR(1, 0)}, // s_mad24 in opencl_std_100
    {168,IR(33, 3),IR(56708, 8),IR(1, 0)}, // u_mad24 in opencl_std_100
    {169,IR(36, 2),IR(56716, 8),IR(1, 0)}, // s_mul24 in opencl_std_100
    {170,IR(36, 2),IR(56724, 8),IR(1, 0)}, // u_mul24 in opencl_std_100
    {171,IR(1144, 3),IR(56732, 7),IR(1, 0)}, // vloadn in opencl_std_100
    {172,IR(33, 3),IR(56739, 8),IR(1, 0)}, // vstoren in opencl_std_100
    {173,IR(36, 2),IR(56747, 11),IR(1, 0)}, // vload_half in opencl_std_100
    {174,IR(1144, 3),IR(56758, 12),IR(1, 0)}, // vload_halfn in opencl_std_100
    {175,IR(33, 3),IR(56770, 12),IR(1, 0)}, // vstore_half in opencl_std_100
    {176,IR(1147, 4),IR(56782, 14),IR(1, 0)}, // vstore_half_r in opencl_std_100
    {177,IR(33, 3),IR(56796, 13),IR(1, 0)}, // vstore_halfn in opencl_std_100
    {178,IR(1147, 4),IR(56809, 15),IR(1, 0)}, // vstore_halfn_r in opencl_std_100
    {179,IR(1144, 3),IR(56824, 13),IR(1, 0)}, // vloada_halfn in opencl_std_100
    {180,IR(33, 3),IR(56837, 14),IR(1, 0)}, // vstorea_halfn in opencl_std_100
    {181,IR(1147, 4),IR(56851, 16),IR(1, 0)}, // vstorea_halfn_r in opencl_std_100
    {182,IR(36, 2),IR(56867, 8),IR(1, 0)}, // shuffle in opencl_std_100
    {183,IR(33, 3),IR(56875, 9),IR(1, 0)}, // shuffle2 in opencl_std_100
    {184,IR(168, 2),IR(56884, 7),IR(1, 0)}, // printf in opencl_std_100
    {185,IR(36, 2),IR(56891, 9),IR(1, 0)}, // prefetch in opencl_std_100
    {186,IR(33, 3),IR(56900, 10),IR(1, 0)}, // bitselect in opencl_std_100
    {187,IR(33, 3),IR(56910, 7),IR(1, 0)}, // select in opencl_std_100
    {201,IR(10, 1),IR(56917, 6),IR(1, 0)}, // u_abs in opencl_std_100
    {202,IR(36, 2),IR(56923, 11),IR(1, 0)}, // u_abs_diff in opencl_std_100
    {203,IR(36, 2),IR(56934, 9),IR(1, 0)}, // u_mul_hi in opencl_std_100
    {204,IR(33, 3),IR(56943, 9),IR(1, 0)}, // u_mad_hi in opencl_std_100
    {1,IR(10, 1),IR(56952, 17),IR(1, 0)}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {2,IR(10, 1),IR(56969, 17),IR(1, 0)}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {3,IR(0, 0),IR(56986, 8),IR(1, 0)}, // TimeAMD in spv_amd_gcn_shader
    {1,IR(36, 2),IR(56994, 22),IR(1, 0)}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {2,IR(36, 2),IR(57016, 28),IR(1, 0)}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {3,IR(33, 3),IR(57044, 19),IR(1, 0)}, // WriteInvocationAMD in spv_amd_shader_ballot
    {4,IR(10, 1),IR(57063, 9),IR(1, 0)}, // MbcntAMD in spv_amd_shader_ballot
    {1,IR(36, 2),IR(57072, 23),IR(1, 0)}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {1,IR(33, 3),IR(57095, 9),IR(1, 0)}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {2,IR(33, 3),IR(57104, 9),IR(1, 0)}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {3,IR(33, 3),IR(57113, 9),IR(1, 0)}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {4,IR(33, 3),IR(57122, 9),IR(1, 0)}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {5,IR(33, 3),IR(57131, 9),IR(1, 0)}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {6,IR(33, 3),IR(57140, 9),IR(1, 0)}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {7,IR(33, 3),IR(57149, 9),IR(1, 0)}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {8,IR(33, 3),IR(57158, 9),IR(1, 0)}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {9,IR(33, 3),IR(57167, 9),IR(1, 0)}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {0,IR(33, 3),IR(57176, 7),IR(1, 0)}, // ARGMAX in tosa_001000_1
    {1,IR(516, 7),IR(57183, 11),IR(1, 0)}, // AVG_POOL2D in tosa_001000_1
    {2,IR(755, 10),IR(57194, 7),IR(1, 0)}, // CONV2D in tosa_001000_1
    {3,IR(755, 10),IR(57201, 7),IR(1, 0)}, // CONV3D in tosa_001000_1
    {4,IR(755, 10),IR(57208, 17),IR(1, 0)}, // DEPTHWISE_CONV2D in tosa_001000_1
    {5,IR(296, 4),IR(57225, 6),IR(1, 0)}, // FFT2D in tosa_001000_1
    {6,IR(296, 4),IR(57231, 7),IR(1, 0)}, // MATMUL in tosa_001000_1
    {7,IR(675, 5),IR(57238, 11),IR(1, 0)}, // MAX_POOL2D in tosa_001000_1
    {8,IR(36, 2),IR(57249, 7),IR(1, 0)}, // RFFT2D in tosa_001000_1
    {9,IR(765, 9),IR(57256, 17),IR(1, 0)}, // TRANSPOSE_CONV2D in tosa_001000_1
    {10,IR(296, 4),IR(57273, 6),IR(1, 0)}, // CLAMP in tosa_001000_1
    {11,IR(10, 1),IR(57279, 4),IR(1, 0)}, // ERF in tosa_001000_1
    {12,IR(10, 1),IR(57283, 8),IR(1, 0)}, // SIGMOID in tosa_001000_1
    {13,IR(10, 1),IR(57291, 5),IR(1, 0)}, // TANH in tosa_001000_1
    {14,IR(36, 2),IR(57296, 4),IR(1, 0)}, // ADD in tosa_001000_1
    {15,IR(33, 3),IR(57300, 23),IR(1, 0)}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {16,IR(36, 2),IR(57323, 12),IR(1, 0)}, // BITWISE_AND in tosa_001000_1
    {17,IR(36, 2),IR(57335, 11),IR(1, 0)}, // BITWISE_OR in tosa_001000_1
    {18,IR(36, 2),IR(57346, 12),IR(1, 0)}, // BITWISE_XOR in tosa_001000_1
    {19,IR(36, 2),IR(57358, 7),IR(1, 0)}, // INTDIV in tosa_001000_1
    {20,IR(36, 2),IR(57365, 12),IR(1, 0)}, // LOGICAL_AND in tosa_001000_1
    {21,IR(36, 2),IR(57377, 19),IR(1, 0)}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {22,IR(36, 2),IR(57396, 20),IR(1, 0)}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {23,IR(36, 2),IR(57416, 11),IR(1, 0)}, // LOGICAL_OR in tosa_001000_1
    {24,IR(36, 2),IR(57427, 12),IR(1, 0)}, // LOGICAL_XOR in tosa_001000_1
    {25,IR(33, 3),IR(57439, 8),IR(1, 0)}, // MAXIMUM in tosa_001000_1
    {26,IR(33, 3),IR(57447, 8),IR(1, 0)}, // MINIMUM in tosa_001000_1
    {27,IR(33, 3),IR(57455, 4),IR(1, 0)}, // MUL in tosa_001000_1
    {28,IR(36, 2),IR(57459, 4),IR(1, 0)}, // POW in tosa_001000_1
    {29,IR(36, 2),IR(57463, 4),IR(1, 0)}, // SUB in tosa_001000_1
    {30,IR(36, 2),IR(57467, 6),IR(1, 0)}, // TABLE in tosa_001000_1
    {31,IR(10, 1),IR(57473, 4),IR(1, 0)}, // ABS in tosa_001000_1
    {32,IR(10, 1),IR(57477, 12),IR(1, 0)}, // BITWISE_NOT in tosa_001000_1
    {33,IR(10, 1),IR(57489, 5),IR(1, 0)}, // CEIL in tosa_001000_1
    {34,IR(10, 1),IR(57494, 4),IR(1, 0)}, // CLZ in tosa_001000_1
    {35,IR(10, 1),IR(57498, 4),IR(1, 0)}, // COS in tosa_001000_1
    {36,IR(10, 1),IR(57502, 4),IR(1, 0)}, // EXP in tosa_001000_1
    {37,IR(10, 1),IR(57506, 6),IR(1, 0)}, // FLOOR in tosa_001000_1
    {38,IR(10, 1),IR(57512, 4),IR(1, 0)}, // LOG in tosa_001000_1
    {39,IR(10, 1),IR(57516, 12),IR(1, 0)}, // LOGICAL_NOT in tosa_001000_1
    {40,IR(33, 3),IR(57528, 7),IR(1, 0)}, // NEGATE in tosa_001000_1
    {41,IR(10, 1),IR(57535, 11),IR(1, 0)}, // RECIPROCAL in tosa_001000_1
    {42,IR(10, 1),IR(57546, 6),IR(1, 0)}, // RSQRT in tosa_001000_1
    {43,IR(10, 1),IR(57552, 4),IR(1, 0)}, // SIN in tosa_001000_1
    {44,IR(33, 3),IR(57556, 7),IR(1, 0)}, // SELECT in tosa_001000_1
    {45,IR(36, 2),IR(57563, 6),IR(1, 0)}, // EQUAL in tosa_001000_1
    {46,IR(36, 2),IR(57569, 8),IR(1, 0)}, // GREATER in tosa_001000_1
    {47,IR(36, 2),IR(57577, 14),IR(1, 0)}, // GREATER_EQUAL in tosa_001000_1
    {48,IR(36, 2),IR(57591, 11),IR(1, 0)}, // REDUCE_ALL in tosa_001000_1
    {49,IR(36, 2),IR(57602, 11),IR(1, 0)}, // REDUCE_ANY in tosa_001000_1
    {50,IR(33, 3),IR(57613, 11),IR(1, 0)}, // REDUCE_MAX in tosa_001000_1
    {51,IR(33, 3),IR(57624, 11),IR(1, 0)}, // REDUCE_MIN in tosa_001000_1
    {52,IR(36, 2),IR(57635, 15),IR(1, 0)}, // REDUCE_PRODUCT in tosa_001000_1
    {53,IR(36, 2),IR(57650, 11),IR(1, 0)}, // REDUCE_SUM in tosa_001000_1
    {54,IR(168, 2),IR(57661, 7),IR(1, 0)}, // CONCAT in tosa_001000_1
    {55,IR(33, 3),IR(57668, 4),IR(1, 0)}, // PAD in tosa_001000_1
    {56,IR(36, 2),IR(57672, 8),IR(1, 0)}, // RESHAPE in tosa_001000_1
    {57,IR(36, 2),IR(57680, 8),IR(1, 0)}, // REVERSE in tosa_001000_1
    {58,IR(33, 3),IR(57688, 6),IR(1, 0)}, // SLICE in tosa_001000_1
    {59,IR(36, 2),IR(57694, 5),IR(1, 0)}, // TILE in tosa_001000_1
    {60,IR(36, 2),IR(57699, 10),IR(1, 0)}, // TRANSPOSE in tosa_001000_1
    {61,IR(36, 2),IR(57709, 7),IR(1, 0)}, // GATHER in tosa_001000_1
    {62,IR(33, 3),IR(57716, 8),IR(1, 0)}, // SCATTER in tosa_001000_1
    {63,IR(675, 5),IR(57724, 7),IR(1, 0)}, // RESIZE in tosa_001000_1
    {64,IR(10, 1),IR(57731, 5),IR(1, 0)}, // CAST in tosa_001000_1
    {65,IR(755, 10),IR(57736, 8),IR(1, 0)}, // RESCALE in tosa_001000_1
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
    {IR(52309, 5), 0}, // CALL in arm_experimental_ml_operations
    {IR(52314, 8), 1}, // MIN_SAD in arm_motion_engine_100
    {IR(52322, 13), 2}, // MIN_SAD_COST in arm_motion_engine_100
    {IR(52335, 8), 3}, // RAW_SAD in arm_motion_engine_100
    {IR(52357, 21), 5}, // DebugCompilationUnit in debuginfo
    {IR(53084, 13), 32}, // DebugDeclare in debuginfo
    {IR(53156, 16), 35}, // DebugExpression in debuginfo
    {IR(52900, 14), 24}, // DebugFunction in debuginfo
    {IR(52875, 25), 23}, // DebugFunctionDeclaration in debuginfo
    {IR(52855, 20), 22}, // DebugGlobalVariable in debuginfo
    {IR(52343, 14), 4}, // DebugInfoNone in debuginfo
    {IR(52987, 15), 29}, // DebugInlinedAt in debuginfo
    {IR(53063, 21), 31}, // DebugInlinedVariable in debuginfo
    {IR(52914, 18), 25}, // DebugLexicalBlock in debuginfo
    {IR(52932, 31), 26}, // DebugLexicalBlockDiscriminator in debuginfo
    {IR(53044, 19), 30}, // DebugLocalVariable in debuginfo
    {IR(53172, 14), 36}, // DebugMacroDef in debuginfo
    {IR(53186, 16), 37}, // DebugMacroUndef in debuginfo
    {IR(52974, 13), 28}, // DebugNoScope in debuginfo
    {IR(53141, 15), 34}, // DebugOperation in debuginfo
    {IR(52963, 11), 27}, // DebugScope in debuginfo
    {IR(52553, 15), 9}, // DebugTypeArray in debuginfo
    {IR(52430, 15), 6}, // DebugTypeBasic in debuginfo
    {IR(52667, 19), 14}, // DebugTypeComposite in debuginfo
    {IR(52615, 14), 13}, // DebugTypeEnum in debuginfo
    {IR(52597, 18), 12}, // DebugTypeFunction in debuginfo
    {IR(52702, 21), 16}, // DebugTypeInheritance in debuginfo
    {IR(52686, 16), 15}, // DebugTypeMember in debuginfo
    {IR(52479, 17), 7}, // DebugTypePointer in debuginfo
    {IR(52723, 21), 17}, // DebugTypePtrToMember in debuginfo
    {IR(52534, 19), 8}, // DebugTypeQualifier in debuginfo
    {IR(52744, 18), 18}, // DebugTypeTemplate in debuginfo
    {IR(52762, 27), 19}, // DebugTypeTemplateParameter in debuginfo
    {IR(52824, 31), 21}, // DebugTypeTemplateParameterPack in debuginfo
    {IR(52789, 35), 20}, // DebugTypeTemplateTemplateParameter in debuginfo
    {IR(52568, 16), 10}, // DebugTypeVector in debuginfo
    {IR(52584, 13), 11}, // DebugTypedef in debuginfo
    {IR(53097, 11), 33}, // DebugValue in debuginfo
    {IR(53296, 5), 54}, // Acos in glsl_std_450
    {IR(53327, 6), 60}, // Acosh in glsl_std_450
    {IR(53291, 5), 53}, // Asin in glsl_std_450
    {IR(53321, 6), 59}, // Asinh in glsl_std_450
    {IR(53301, 5), 55}, // Atan in glsl_std_450
    {IR(53339, 6), 62}, // Atan2 in glsl_std_450
    {IR(53333, 6), 61}, // Atanh in glsl_std_450
    {IR(53252, 5), 46}, // Ceil in glsl_std_450
    {IR(53283, 4), 51}, // Cos in glsl_std_450
    {IR(53311, 5), 57}, // Cosh in glsl_std_450
    {IR(53723, 6), 105}, // Cross in glsl_std_450
    {IR(53271, 8), 49}, // Degrees in glsl_std_450
    {IR(53384, 12), 70}, // Determinant in glsl_std_450
    {IR(53714, 9), 104}, // Distance in glsl_std_450
    {IR(53349, 4), 64}, // Exp in glsl_std_450
    {IR(53357, 5), 66}, // Exp2 in glsl_std_450
    {IR(53224, 5), 41}, // FAbs in glsl_std_450
    {IR(53456, 7), 80}, // FClamp in glsl_std_450
    {IR(53441, 5), 77}, // FMax in glsl_std_450
    {IR(53426, 5), 74}, // FMin in glsl_std_450
    {IR(53477, 5), 83}, // FMix in glsl_std_450
    {IR(53234, 6), 43}, // FSign in glsl_std_450
    {IR(53739, 12), 107}, // FaceForward in glsl_std_450
    {IR(53767, 9), 110}, // FindILsb in glsl_std_450
    {IR(53776, 9), 111}, // FindSMsb in glsl_std_450
    {IR(53785, 9), 112}, // FindUMsb in glsl_std_450
    {IR(53246, 6), 45}, // Floor in glsl_std_450
    {IR(53503, 4), 87}, // Fma in glsl_std_450
    {IR(53257, 6), 47}, // Fract in glsl_std_450
    {IR(53507, 6), 88}, // Frexp in glsl_std_450
    {IR(53513, 12), 89}, // FrexpStruct in glsl_std_450
    {IR(53482, 5), 84}, // IMix in glsl_std_450
    {IR(53794, 22), 113}, // InterpolateAtCentroid in glsl_std_450
    {IR(53836, 20), 115}, // InterpolateAtOffset in glsl_std_450
    {IR(53816, 20), 114}, // InterpolateAtSample in glsl_std_450
    {IR(53372, 12), 69}, // InverseSqrt in glsl_std_450
    {IR(53525, 6), 90}, // Ldexp in glsl_std_450
    {IR(53707, 7), 103}, // Length in glsl_std_450
    {IR(53353, 4), 65}, // Log in glsl_std_450
    {IR(53362, 5), 67}, // Log2 in glsl_std_450
    {IR(53396, 14), 71}, // MatrixInverse in glsl_std_450
    {IR(53410, 5), 72}, // Modf in glsl_std_450
    {IR(53415, 11), 73}, // ModfStruct in glsl_std_450
    {IR(53866, 7), 118}, // NClamp in glsl_std_450
    {IR(53861, 5), 117}, // NMax in glsl_std_450
    {IR(53856, 5), 116}, // NMin in glsl_std_450
    {IR(53729, 10), 106}, // Normalize in glsl_std_450
    {IR(53598, 15), 96}, // PackDouble2x32 in glsl_std_450
    {IR(53585, 13), 95}, // PackHalf2x16 in glsl_std_450
    {IR(53557, 14), 93}, // PackSnorm2x16 in glsl_std_450
    {IR(53531, 13), 91}, // PackSnorm4x8 in glsl_std_450
    {IR(53571, 14), 94}, // PackUnorm2x16 in glsl_std_450
    {IR(53544, 13), 92}, // PackUnorm4x8 in glsl_std_450
    {IR(53345, 4), 63}, // Pow in glsl_std_450
    {IR(53263, 8), 48}, // Radians in glsl_std_450
    {IR(53751, 8), 108}, // Reflect in glsl_std_450
    {IR(53759, 8), 109}, // Refract in glsl_std_450
    {IR(53202, 6), 38}, // Round in glsl_std_450
    {IR(53208, 10), 39}, // RoundEven in glsl_std_450
    {IR(53229, 5), 42}, // SAbs in glsl_std_450
    {IR(53470, 7), 82}, // SClamp in glsl_std_450
    {IR(53451, 5), 79}, // SMax in glsl_std_450
    {IR(53436, 5), 76}, // SMin in glsl_std_450
    {IR(53240, 6), 44}, // SSign in glsl_std_450
    {IR(53279, 4), 50}, // Sin in glsl_std_450
    {IR(53306, 5), 56}, // Sinh in glsl_std_450
    {IR(53492, 11), 86}, // SmoothStep in glsl_std_450
    {IR(53367, 5), 68}, // Sqrt in glsl_std_450
    {IR(53487, 5), 85}, // Step in glsl_std_450
    {IR(53287, 4), 52}, // Tan in glsl_std_450
    {IR(53316, 5), 58}, // Tanh in glsl_std_450
    {IR(53218, 6), 40}, // Trunc in glsl_std_450
    {IR(53463, 7), 81}, // UClamp in glsl_std_450
    {IR(53446, 5), 78}, // UMax in glsl_std_450
    {IR(53431, 5), 75}, // UMin in glsl_std_450
    {IR(53690, 17), 102}, // UnpackDouble2x32 in glsl_std_450
    {IR(53645, 15), 99}, // UnpackHalf2x16 in glsl_std_450
    {IR(53613, 16), 97}, // UnpackSnorm2x16 in glsl_std_450
    {IR(53660, 15), 100}, // UnpackSnorm4x8 in glsl_std_450
    {IR(53629, 16), 98}, // UnpackUnorm2x16 in glsl_std_450
    {IR(53675, 15), 101}, // UnpackUnorm4x8 in glsl_std_450
    {IR(53873, 13), 120}, // ArgumentInfo in nonsemantic_clspvreflection
    {IR(53968, 24), 125}, // ArgumentPodPushConstant in nonsemantic_clspvreflection
    {IR(53924, 25), 123}, // ArgumentPodStorageBuffer in nonsemantic_clspvreflection
    {IR(53949, 19), 124}, // ArgumentPodUniform in nonsemantic_clspvreflection
    {IR(54402, 28), 144}, // ArgumentPointerPushConstant in nonsemantic_clspvreflection
    {IR(54430, 23), 145}, // ArgumentPointerUniform in nonsemantic_clspvreflection
    {IR(53992, 21), 126}, // ArgumentSampledImage in nonsemantic_clspvreflection
    {IR(54034, 16), 128}, // ArgumentSampler in nonsemantic_clspvreflection
    {IR(53886, 22), 121}, // ArgumentStorageBuffer in nonsemantic_clspvreflection
    {IR(54013, 21), 127}, // ArgumentStorageImage in nonsemantic_clspvreflection
    {IR(54690, 27), 152}, // ArgumentStorageTexelBuffer in nonsemantic_clspvreflection
    {IR(53908, 16), 122}, // ArgumentUniform in nonsemantic_clspvreflection
    {IR(54717, 27), 153}, // ArgumentUniformTexelBuffer in nonsemantic_clspvreflection
    {IR(54050, 18), 129}, // ArgumentWorkgroup in nonsemantic_clspvreflection
    {IR(54744, 32), 154}, // ConstantDataPointerPushConstant in nonsemantic_clspvreflection
    {IR(54298, 26), 139}, // ConstantDataStorageBuffer in nonsemantic_clspvreflection
    {IR(54324, 20), 140}, // ConstantDataUniform in nonsemantic_clspvreflection
    {IR(54568, 45), 149}, // ImageArgumentInfoChannelDataTypePushConstant in nonsemantic_clspvreflection
    {IR(54650, 40), 151}, // ImageArgumentInfoChannelDataTypeUniform in nonsemantic_clspvreflection
    {IR(54526, 42), 148}, // ImageArgumentInfoChannelOrderPushConstant in nonsemantic_clspvreflection
    {IR(54613, 37), 150}, // ImageArgumentInfoChannelOrderUniform in nonsemantic_clspvreflection
    {IR(5940, 7), 119}, // Kernel in nonsemantic_clspvreflection
    {IR(9457, 15), 141}, // LiteralSampler in nonsemantic_clspvreflection
    {IR(54885, 34), 159}, // NormalizedSamplerMaskPushConstant in nonsemantic_clspvreflection
    {IR(54853, 32), 158}, // PrintfBufferPointerPushConstant in nonsemantic_clspvreflection
    {IR(54827, 26), 157}, // PrintfBufferStorageBuffer in nonsemantic_clspvreflection
    {IR(54816, 11), 156}, // PrintfInfo in nonsemantic_clspvreflection
    {IR(54776, 40), 155}, // ProgramScopeVariablePointerPushConstant in nonsemantic_clspvreflection
    {IR(54488, 38), 147}, // ProgramScopeVariablePointerRelocation in nonsemantic_clspvreflection
    {IR(54453, 35), 146}, // ProgramScopeVariablesStorageBuffer in nonsemantic_clspvreflection
    {IR(54344, 30), 142}, // PropertyRequiredWorkgroupSize in nonsemantic_clspvreflection
    {IR(54164, 30), 134}, // PushConstantEnqueuedLocalSize in nonsemantic_clspvreflection
    {IR(54139, 25), 133}, // PushConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(54194, 23), 135}, // PushConstantGlobalSize in nonsemantic_clspvreflection
    {IR(54242, 26), 137}, // PushConstantNumWorkgroups in nonsemantic_clspvreflection
    {IR(54268, 30), 138}, // PushConstantRegionGroupOffset in nonsemantic_clspvreflection
    {IR(54217, 25), 136}, // PushConstantRegionOffset in nonsemantic_clspvreflection
    {IR(54094, 25), 131}, // SpecConstantGlobalOffset in nonsemantic_clspvreflection
    {IR(54374, 28), 143}, // SpecConstantSubgroupMaxSize in nonsemantic_clspvreflection
    {IR(54119, 20), 132}, // SpecConstantWorkDim in nonsemantic_clspvreflection
    {IR(54068, 26), 130}, // SpecConstantWorkgroupSize in nonsemantic_clspvreflection
    {IR(54919, 22), 160}, // WorkgroupVariableSize in nonsemantic_clspvreflection
    {IR(54941, 12), 161}, // DebugPrintf in nonsemantic_debugprintf
    {IR(54953, 11), 162}, // DebugGraph in nonsemantic_graph_debuginfo
    {IR(53141, 15), 163}, // DebugOperation in nonsemantic_graph_debuginfo
    {IR(54964, 12), 164}, // DebugTensor in nonsemantic_graph_debuginfo
    {IR(55075, 21), 205}, // DebugBuildIdentifier in nonsemantic_shader_debuginfo
    {IR(52357, 21), 166}, // DebugCompilationUnit in nonsemantic_shader_debuginfo
    {IR(53084, 13), 193}, // DebugDeclare in nonsemantic_shader_debuginfo
    {IR(55113, 16), 207}, // DebugEntryPoint in nonsemantic_shader_debuginfo
    {IR(53156, 16), 196}, // DebugExpression in nonsemantic_shader_debuginfo
    {IR(52900, 14), 185}, // DebugFunction in nonsemantic_shader_debuginfo
    {IR(52875, 25), 184}, // DebugFunctionDeclaration in nonsemantic_shader_debuginfo
    {IR(55008, 24), 201}, // DebugFunctionDefinition in nonsemantic_shader_debuginfo
    {IR(52855, 20), 183}, // DebugGlobalVariable in nonsemantic_shader_debuginfo
    {IR(54976, 20), 199}, // DebugImportedEntity in nonsemantic_shader_debuginfo
    {IR(52343, 14), 165}, // DebugInfoNone in nonsemantic_shader_debuginfo
    {IR(52987, 15), 190}, // DebugInlinedAt in nonsemantic_shader_debuginfo
    {IR(53063, 21), 192}, // DebugInlinedVariable in nonsemantic_shader_debuginfo
    {IR(52914, 18), 186}, // DebugLexicalBlock in nonsemantic_shader_debuginfo
    {IR(52932, 31), 187}, // DebugLexicalBlockDiscriminator in nonsemantic_shader_debuginfo
    {IR(55053, 10), 203}, // DebugLine in nonsemantic_shader_debuginfo
    {IR(53044, 19), 191}, // DebugLocalVariable in nonsemantic_shader_debuginfo
    {IR(53172, 14), 197}, // DebugMacroDef in nonsemantic_shader_debuginfo
    {IR(53186, 16), 198}, // DebugMacroUndef in nonsemantic_shader_debuginfo
    {IR(55063, 12), 204}, // DebugNoLine in nonsemantic_shader_debuginfo
    {IR(52974, 13), 189}, // DebugNoScope in nonsemantic_shader_debuginfo
    {IR(53141, 15), 195}, // DebugOperation in nonsemantic_shader_debuginfo
    {IR(52963, 11), 188}, // DebugScope in nonsemantic_shader_debuginfo
    {IR(54996, 12), 200}, // DebugSource in nonsemantic_shader_debuginfo
    {IR(55032, 21), 202}, // DebugSourceContinued in nonsemantic_shader_debuginfo
    {IR(55096, 17), 206}, // DebugStoragePath in nonsemantic_shader_debuginfo
    {IR(52553, 15), 170}, // DebugTypeArray in nonsemantic_shader_debuginfo
    {IR(52430, 15), 167}, // DebugTypeBasic in nonsemantic_shader_debuginfo
    {IR(52667, 19), 175}, // DebugTypeComposite in nonsemantic_shader_debuginfo
    {IR(55166, 30), 210}, // DebugTypeCooperativeMatrixKHR in nonsemantic_shader_debuginfo
    {IR(52615, 14), 174}, // DebugTypeEnum in nonsemantic_shader_debuginfo
    {IR(52597, 18), 173}, // DebugTypeFunction in nonsemantic_shader_debuginfo
    {IR(52702, 21), 177}, // DebugTypeInheritance in nonsemantic_shader_debuginfo
    {IR(55129, 16), 208}, // DebugTypeMatrix in nonsemantic_shader_debuginfo
    {IR(52686, 16), 176}, // DebugTypeMember in nonsemantic_shader_debuginfo
    {IR(52479, 17), 168}, // DebugTypePointer in nonsemantic_shader_debuginfo
    {IR(52723, 21), 178}, // DebugTypePtrToMember in nonsemantic_shader_debuginfo
    {IR(52534, 19), 169}, // DebugTypeQualifier in nonsemantic_shader_debuginfo
    {IR(52744, 18), 179}, // DebugTypeTemplate in nonsemantic_shader_debuginfo
    {IR(52762, 27), 180}, // DebugTypeTemplateParameter in nonsemantic_shader_debuginfo
    {IR(52824, 31), 182}, // DebugTypeTemplateParameterPack in nonsemantic_shader_debuginfo
    {IR(52789, 35), 181}, // DebugTypeTemplateTemplateParameter in nonsemantic_shader_debuginfo
    {IR(52568, 16), 171}, // DebugTypeVector in nonsemantic_shader_debuginfo
    {IR(55145, 21), 209}, // DebugTypeVectorIdEXT in nonsemantic_shader_debuginfo
    {IR(52584, 13), 172}, // DebugTypedef in nonsemantic_shader_debuginfo
    {IR(53097, 11), 194}, // DebugValue in nonsemantic_shader_debuginfo
    {IR(55196, 14), 211}, // Configuration in nonsemantic_vkspreflection
    {IR(55272, 20), 216}, // DescriptorSetBuffer in nonsemantic_vkspreflection
    {IR(55292, 19), 217}, // DescriptorSetImage in nonsemantic_vkspreflection
    {IR(55311, 21), 218}, // DescriptorSetSampler in nonsemantic_vkspreflection
    {IR(55235, 14), 214}, // PushConstants in nonsemantic_vkspreflection
    {IR(55249, 23), 215}, // SpecializationMapEntry in nonsemantic_vkspreflection
    {IR(55210, 13), 212}, // StartCounter in nonsemantic_vkspreflection
    {IR(55223, 12), 213}, // StopCounter in nonsemantic_vkspreflection
    {IR(52357, 21), 220}, // DebugCompilationUnit in opencl_debuginfo_100
    {IR(53084, 13), 247}, // DebugDeclare in opencl_debuginfo_100
    {IR(53156, 16), 250}, // DebugExpression in opencl_debuginfo_100
    {IR(52900, 14), 239}, // DebugFunction in opencl_debuginfo_100
    {IR(52875, 25), 238}, // DebugFunctionDeclaration in opencl_debuginfo_100
    {IR(52855, 20), 237}, // DebugGlobalVariable in opencl_debuginfo_100
    {IR(54976, 20), 253}, // DebugImportedEntity in opencl_debuginfo_100
    {IR(52343, 14), 219}, // DebugInfoNone in opencl_debuginfo_100
    {IR(52987, 15), 244}, // DebugInlinedAt in opencl_debuginfo_100
    {IR(53063, 21), 246}, // DebugInlinedVariable in opencl_debuginfo_100
    {IR(52914, 18), 240}, // DebugLexicalBlock in opencl_debuginfo_100
    {IR(52932, 31), 241}, // DebugLexicalBlockDiscriminator in opencl_debuginfo_100
    {IR(53044, 19), 245}, // DebugLocalVariable in opencl_debuginfo_100
    {IR(53172, 14), 251}, // DebugMacroDef in opencl_debuginfo_100
    {IR(53186, 16), 252}, // DebugMacroUndef in opencl_debuginfo_100
    {IR(55632, 17), 255}, // DebugModuleINTEL in opencl_debuginfo_100
    {IR(52974, 13), 243}, // DebugNoScope in opencl_debuginfo_100
    {IR(53141, 15), 249}, // DebugOperation in opencl_debuginfo_100
    {IR(52963, 11), 242}, // DebugScope in opencl_debuginfo_100
    {IR(54996, 12), 254}, // DebugSource in opencl_debuginfo_100
    {IR(52553, 15), 224}, // DebugTypeArray in opencl_debuginfo_100
    {IR(52430, 15), 221}, // DebugTypeBasic in opencl_debuginfo_100
    {IR(52667, 19), 229}, // DebugTypeComposite in opencl_debuginfo_100
    {IR(52615, 14), 228}, // DebugTypeEnum in opencl_debuginfo_100
    {IR(52597, 18), 227}, // DebugTypeFunction in opencl_debuginfo_100
    {IR(52702, 21), 231}, // DebugTypeInheritance in opencl_debuginfo_100
    {IR(52686, 16), 230}, // DebugTypeMember in opencl_debuginfo_100
    {IR(52479, 17), 222}, // DebugTypePointer in opencl_debuginfo_100
    {IR(52723, 21), 232}, // DebugTypePtrToMember in opencl_debuginfo_100
    {IR(52534, 19), 223}, // DebugTypeQualifier in opencl_debuginfo_100
    {IR(52744, 18), 233}, // DebugTypeTemplate in opencl_debuginfo_100
    {IR(52762, 27), 234}, // DebugTypeTemplateParameter in opencl_debuginfo_100
    {IR(52824, 31), 236}, // DebugTypeTemplateParameterPack in opencl_debuginfo_100
    {IR(52789, 35), 235}, // DebugTypeTemplateTemplateParameter in opencl_debuginfo_100
    {IR(52568, 16), 225}, // DebugTypeVector in opencl_debuginfo_100
    {IR(52584, 13), 226}, // DebugTypedef in opencl_debuginfo_100
    {IR(53097, 11), 248}, // DebugValue in opencl_debuginfo_100
    {IR(55649, 5), 256}, // acos in opencl_std_100
    {IR(55654, 6), 257}, // acosh in opencl_std_100
    {IR(55660, 7), 258}, // acospi in opencl_std_100
    {IR(55667, 5), 259}, // asin in opencl_std_100
    {IR(55672, 6), 260}, // asinh in opencl_std_100
    {IR(55678, 7), 261}, // asinpi in opencl_std_100
    {IR(55685, 5), 262}, // atan in opencl_std_100
    {IR(55690, 6), 263}, // atan2 in opencl_std_100
    {IR(55709, 8), 266}, // atan2pi in opencl_std_100
    {IR(55696, 6), 264}, // atanh in opencl_std_100
    {IR(55702, 7), 265}, // atanpi in opencl_std_100
    {IR(56900, 10), 412}, // bitselect in opencl_std_100
    {IR(55717, 5), 267}, // cbrt in opencl_std_100
    {IR(55722, 5), 268}, // ceil in opencl_std_100
    {IR(56572, 4), 377}, // clz in opencl_std_100
    {IR(55727, 9), 269}, // copysign in opencl_std_100
    {IR(55736, 4), 270}, // cos in opencl_std_100
    {IR(55740, 5), 271}, // cosh in opencl_std_100
    {IR(55745, 6), 272}, // cospi in opencl_std_100
    {IR(56416, 6), 360}, // cross in opencl_std_100
    {IR(56576, 4), 378}, // ctz in opencl_std_100
    {IR(56351, 8), 352}, // degrees in opencl_std_100
    {IR(56422, 9), 361}, // distance in opencl_std_100
    {IR(55756, 4), 274}, // erf in opencl_std_100
    {IR(55751, 5), 273}, // erfc in opencl_std_100
    {IR(55760, 4), 275}, // exp in opencl_std_100
    {IR(55769, 6), 277}, // exp10 in opencl_std_100
    {IR(55764, 5), 276}, // exp2 in opencl_std_100
    {IR(55775, 6), 278}, // expm1 in opencl_std_100
    {IR(55781, 5), 279}, // fabs in opencl_std_100
    {IR(56448, 14), 364}, // fast_distance in opencl_std_100
    {IR(56462, 12), 365}, // fast_length in opencl_std_100
    {IR(56474, 15), 366}, // fast_normalize in opencl_std_100
    {IR(56344, 7), 351}, // fclamp in opencl_std_100
    {IR(55786, 5), 280}, // fdim in opencl_std_100
    {IR(55791, 6), 281}, // floor in opencl_std_100
    {IR(55797, 4), 282}, // fma in opencl_std_100
    {IR(55801, 5), 283}, // fmax in opencl_std_100
    {IR(56359, 12), 353}, // fmax_common in opencl_std_100
    {IR(55806, 5), 284}, // fmin in opencl_std_100
    {IR(56371, 12), 354}, // fmin_common in opencl_std_100
    {IR(55811, 5), 285}, // fmod in opencl_std_100
    {IR(55816, 6), 286}, // fract in opencl_std_100
    {IR(55822, 6), 287}, // frexp in opencl_std_100
    {IR(56034, 9), 323}, // half_cos in opencl_std_100
    {IR(56043, 12), 324}, // half_divide in opencl_std_100
    {IR(56055, 9), 325}, // half_exp in opencl_std_100
    {IR(56074, 11), 327}, // half_exp10 in opencl_std_100
    {IR(56064, 10), 326}, // half_exp2 in opencl_std_100
    {IR(56085, 9), 328}, // half_log in opencl_std_100
    {IR(56104, 11), 330}, // half_log10 in opencl_std_100
    {IR(56094, 10), 329}, // half_log2 in opencl_std_100
    {IR(56115, 10), 331}, // half_powr in opencl_std_100
    {IR(56125, 11), 332}, // half_recip in opencl_std_100
    {IR(56136, 11), 333}, // half_rsqrt in opencl_std_100
    {IR(56147, 9), 334}, // half_sin in opencl_std_100
    {IR(56156, 10), 335}, // half_sqrt in opencl_std_100
    {IR(56166, 9), 336}, // half_tan in opencl_std_100
    {IR(55828, 6), 288}, // hypot in opencl_std_100
    {IR(55834, 6), 289}, // ilogb in opencl_std_100
    {IR(55840, 6), 290}, // ldexp in opencl_std_100
    {IR(56431, 7), 362}, // length in opencl_std_100
    {IR(55846, 7), 291}, // lgamma in opencl_std_100
    {IR(55853, 9), 292}, // lgamma_r in opencl_std_100
    {IR(55862, 4), 293}, // log in opencl_std_100
    {IR(55871, 6), 295}, // log10 in opencl_std_100
    {IR(55877, 6), 296}, // log1p in opencl_std_100
    {IR(55866, 5), 294}, // log2 in opencl_std_100
    {IR(55883, 5), 297}, // logb in opencl_std_100
    {IR(55888, 4), 298}, // mad in opencl_std_100
    {IR(55892, 7), 299}, // maxmag in opencl_std_100
    {IR(55899, 7), 300}, // minmag in opencl_std_100
    {IR(56383, 4), 355}, // mix in opencl_std_100
    {IR(55906, 5), 301}, // modf in opencl_std_100
    {IR(55911, 4), 302}, // nan in opencl_std_100
    {IR(56175, 11), 337}, // native_cos in opencl_std_100
    {IR(56186, 14), 338}, // native_divide in opencl_std_100
    {IR(56200, 11), 339}, // native_exp in opencl_std_100
    {IR(56223, 13), 341}, // native_exp10 in opencl_std_100
    {IR(56211, 12), 340}, // native_exp2 in opencl_std_100
    {IR(56236, 11), 342}, // native_log in opencl_std_100
    {IR(56259, 13), 344}, // native_log10 in opencl_std_100
    {IR(56247, 12), 343}, // native_log2 in opencl_std_100
    {IR(56272, 12), 345}, // native_powr in opencl_std_100
    {IR(56284, 13), 346}, // native_recip in opencl_std_100
    {IR(56297, 13), 347}, // native_rsqrt in opencl_std_100
    {IR(56310, 11), 348}, // native_sin in opencl_std_100
    {IR(56321, 12), 349}, // native_sqrt in opencl_std_100
    {IR(56333, 11), 350}, // native_tan in opencl_std_100
    {IR(55915, 10), 303}, // nextafter in opencl_std_100
    {IR(56438, 10), 363}, // normalize in opencl_std_100
    {IR(56691, 9), 392}, // popcount in opencl_std_100
    {IR(55925, 4), 304}, // pow in opencl_std_100
    {IR(55929, 5), 305}, // pown in opencl_std_100
    {IR(55934, 5), 306}, // powr in opencl_std_100
    {IR(56891, 9), 411}, // prefetch in opencl_std_100
    {IR(56884, 7), 410}, // printf in opencl_std_100
    {IR(56387, 8), 356}, // radians in opencl_std_100
    {IR(55939, 10), 307}, // remainder in opencl_std_100
    {IR(55949, 7), 308}, // remquo in opencl_std_100
    {IR(55956, 5), 309}, // rint in opencl_std_100
    {IR(55961, 6), 310}, // rootn in opencl_std_100
    {IR(56642, 7), 387}, // rotate in opencl_std_100
    {IR(55967, 6), 311}, // round in opencl_std_100
    {IR(55973, 6), 312}, // rsqrt in opencl_std_100
    {IR(56489, 6), 367}, // s_abs in opencl_std_100
    {IR(56495, 11), 368}, // s_abs_diff in opencl_std_100
    {IR(56506, 10), 369}, // s_add_sat in opencl_std_100
    {IR(56556, 8), 375}, // s_clamp in opencl_std_100
    {IR(56526, 7), 371}, // s_hadd in opencl_std_100
    {IR(56700, 8), 393}, // s_mad24 in opencl_std_100
    {IR(56580, 9), 379}, // s_mad_hi in opencl_std_100
    {IR(56599, 10), 381}, // s_mad_sat in opencl_std_100
    {IR(56609, 6), 382}, // s_max in opencl_std_100
    {IR(56621, 6), 384}, // s_min in opencl_std_100
    {IR(56716, 8), 395}, // s_mul24 in opencl_std_100
    {IR(56633, 9), 386}, // s_mul_hi in opencl_std_100
    {IR(56540, 8), 373}, // s_rhadd in opencl_std_100
    {IR(56649, 10), 388}, // s_sub_sat in opencl_std_100
    {IR(56680, 11), 391}, // s_upsample in opencl_std_100
    {IR(56910, 7), 413}, // select in opencl_std_100
    {IR(56867, 8), 408}, // shuffle in opencl_std_100
    {IR(56875, 9), 409}, // shuffle2 in opencl_std_100
    {IR(56411, 5), 359}, // sign in opencl_std_100
    {IR(55979, 4), 313}, // sin in opencl_std_100
    {IR(55983, 7), 314}, // sincos in opencl_std_100
    {IR(55990, 5), 315}, // sinh in opencl_std_100
    {IR(55995, 6), 316}, // sinpi in opencl_std_100
    {IR(56400, 11), 358}, // smoothstep in opencl_std_100
    {IR(56001, 5), 317}, // sqrt in opencl_std_100
    {IR(56395, 5), 357}, // step in opencl_std_100
    {IR(56006, 4), 318}, // tan in opencl_std_100
    {IR(56010, 5), 319}, // tanh in opencl_std_100
    {IR(56015, 6), 320}, // tanpi in opencl_std_100
    {IR(56021, 7), 321}, // tgamma in opencl_std_100
    {IR(56028, 6), 322}, // trunc in opencl_std_100
    {IR(56917, 6), 414}, // u_abs in opencl_std_100
    {IR(56923, 11), 415}, // u_abs_diff in opencl_std_100
    {IR(56516, 10), 370}, // u_add_sat in opencl_std_100
    {IR(56564, 8), 376}, // u_clamp in opencl_std_100
    {IR(56533, 7), 372}, // u_hadd in opencl_std_100
    {IR(56708, 8), 394}, // u_mad24 in opencl_std_100
    {IR(56943, 9), 417}, // u_mad_hi in opencl_std_100
    {IR(56589, 10), 380}, // u_mad_sat in opencl_std_100
    {IR(56615, 6), 383}, // u_max in opencl_std_100
    {IR(56627, 6), 385}, // u_min in opencl_std_100
    {IR(56724, 8), 396}, // u_mul24 in opencl_std_100
    {IR(56934, 9), 416}, // u_mul_hi in opencl_std_100
    {IR(56548, 8), 374}, // u_rhadd in opencl_std_100
    {IR(56659, 10), 389}, // u_sub_sat in opencl_std_100
    {IR(56669, 11), 390}, // u_upsample in opencl_std_100
    {IR(56747, 11), 399}, // vload_half in opencl_std_100
    {IR(56758, 12), 400}, // vload_halfn in opencl_std_100
    {IR(56824, 13), 405}, // vloada_halfn in opencl_std_100
    {IR(56732, 7), 397}, // vloadn in opencl_std_100
    {IR(56770, 12), 401}, // vstore_half in opencl_std_100
    {IR(56782, 14), 402}, // vstore_half_r in opencl_std_100
    {IR(56796, 13), 403}, // vstore_halfn in opencl_std_100
    {IR(56809, 15), 404}, // vstore_halfn_r in opencl_std_100
    {IR(56837, 14), 406}, // vstorea_halfn in opencl_std_100
    {IR(56851, 16), 407}, // vstorea_halfn_r in opencl_std_100
    {IR(56739, 8), 398}, // vstoren in opencl_std_100
    {IR(56969, 17), 419}, // CubeFaceCoordAMD in spv_amd_gcn_shader
    {IR(56952, 17), 418}, // CubeFaceIndexAMD in spv_amd_gcn_shader
    {IR(56986, 8), 420}, // TimeAMD in spv_amd_gcn_shader
    {IR(57063, 9), 424}, // MbcntAMD in spv_amd_shader_ballot
    {IR(56994, 22), 421}, // SwizzleInvocationsAMD in spv_amd_shader_ballot
    {IR(57016, 28), 422}, // SwizzleInvocationsMaskedAMD in spv_amd_shader_ballot
    {IR(57044, 19), 423}, // WriteInvocationAMD in spv_amd_shader_ballot
    {IR(57072, 23), 425}, // InterpolateAtVertexAMD in spv_amd_shader_explicit_vertex_parameter
    {IR(57122, 9), 429}, // FMax3AMD in spv_amd_shader_trinary_minmax
    {IR(57149, 9), 432}, // FMid3AMD in spv_amd_shader_trinary_minmax
    {IR(57095, 9), 426}, // FMin3AMD in spv_amd_shader_trinary_minmax
    {IR(57140, 9), 431}, // SMax3AMD in spv_amd_shader_trinary_minmax
    {IR(57167, 9), 434}, // SMid3AMD in spv_amd_shader_trinary_minmax
    {IR(57113, 9), 428}, // SMin3AMD in spv_amd_shader_trinary_minmax
    {IR(57131, 9), 430}, // UMax3AMD in spv_amd_shader_trinary_minmax
    {IR(57158, 9), 433}, // UMid3AMD in spv_amd_shader_trinary_minmax
    {IR(57104, 9), 427}, // UMin3AMD in spv_amd_shader_trinary_minmax
    {IR(57473, 4), 466}, // ABS in tosa_001000_1
    {IR(57296, 4), 449}, // ADD in tosa_001000_1
    {IR(57176, 7), 435}, // ARGMAX in tosa_001000_1
    {IR(57300, 23), 450}, // ARITHMETIC_RIGHT_SHIFT in tosa_001000_1
    {IR(57183, 11), 436}, // AVG_POOL2D in tosa_001000_1
    {IR(57323, 12), 451}, // BITWISE_AND in tosa_001000_1
    {IR(57477, 12), 467}, // BITWISE_NOT in tosa_001000_1
    {IR(57335, 11), 452}, // BITWISE_OR in tosa_001000_1
    {IR(57346, 12), 453}, // BITWISE_XOR in tosa_001000_1
    {IR(57731, 5), 499}, // CAST in tosa_001000_1
    {IR(57489, 5), 468}, // CEIL in tosa_001000_1
    {IR(57273, 6), 445}, // CLAMP in tosa_001000_1
    {IR(57494, 4), 469}, // CLZ in tosa_001000_1
    {IR(57661, 7), 489}, // CONCAT in tosa_001000_1
    {IR(57194, 7), 437}, // CONV2D in tosa_001000_1
    {IR(57201, 7), 438}, // CONV3D in tosa_001000_1
    {IR(57498, 4), 470}, // COS in tosa_001000_1
    {IR(57208, 17), 439}, // DEPTHWISE_CONV2D in tosa_001000_1
    {IR(57563, 6), 480}, // EQUAL in tosa_001000_1
    {IR(57279, 4), 446}, // ERF in tosa_001000_1
    {IR(57502, 4), 471}, // EXP in tosa_001000_1
    {IR(57225, 6), 440}, // FFT2D in tosa_001000_1
    {IR(57506, 6), 472}, // FLOOR in tosa_001000_1
    {IR(57709, 7), 496}, // GATHER in tosa_001000_1
    {IR(57569, 8), 481}, // GREATER in tosa_001000_1
    {IR(57577, 14), 482}, // GREATER_EQUAL in tosa_001000_1
    {IR(57358, 7), 454}, // INTDIV in tosa_001000_1
    {IR(57512, 4), 473}, // LOG in tosa_001000_1
    {IR(57365, 12), 455}, // LOGICAL_AND in tosa_001000_1
    {IR(57377, 19), 456}, // LOGICAL_LEFT_SHIFT in tosa_001000_1
    {IR(57516, 12), 474}, // LOGICAL_NOT in tosa_001000_1
    {IR(57416, 11), 458}, // LOGICAL_OR in tosa_001000_1
    {IR(57396, 20), 457}, // LOGICAL_RIGHT_SHIFT in tosa_001000_1
    {IR(57427, 12), 459}, // LOGICAL_XOR in tosa_001000_1
    {IR(57231, 7), 441}, // MATMUL in tosa_001000_1
    {IR(57439, 8), 460}, // MAXIMUM in tosa_001000_1
    {IR(57238, 11), 442}, // MAX_POOL2D in tosa_001000_1
    {IR(57447, 8), 461}, // MINIMUM in tosa_001000_1
    {IR(57455, 4), 462}, // MUL in tosa_001000_1
    {IR(57528, 7), 475}, // NEGATE in tosa_001000_1
    {IR(57668, 4), 490}, // PAD in tosa_001000_1
    {IR(57459, 4), 463}, // POW in tosa_001000_1
    {IR(57535, 11), 476}, // RECIPROCAL in tosa_001000_1
    {IR(57591, 11), 483}, // REDUCE_ALL in tosa_001000_1
    {IR(57602, 11), 484}, // REDUCE_ANY in tosa_001000_1
    {IR(57613, 11), 485}, // REDUCE_MAX in tosa_001000_1
    {IR(57624, 11), 486}, // REDUCE_MIN in tosa_001000_1
    {IR(57635, 15), 487}, // REDUCE_PRODUCT in tosa_001000_1
    {IR(57650, 11), 488}, // REDUCE_SUM in tosa_001000_1
    {IR(57736, 8), 500}, // RESCALE in tosa_001000_1
    {IR(57672, 8), 491}, // RESHAPE in tosa_001000_1
    {IR(57724, 7), 498}, // RESIZE in tosa_001000_1
    {IR(57680, 8), 492}, // REVERSE in tosa_001000_1
    {IR(57249, 7), 443}, // RFFT2D in tosa_001000_1
    {IR(57546, 6), 477}, // RSQRT in tosa_001000_1
    {IR(57716, 8), 497}, // SCATTER in tosa_001000_1
    {IR(57556, 7), 479}, // SELECT in tosa_001000_1
    {IR(57283, 8), 447}, // SIGMOID in tosa_001000_1
    {IR(57552, 4), 478}, // SIN in tosa_001000_1
    {IR(57688, 6), 493}, // SLICE in tosa_001000_1
    {IR(57463, 4), 464}, // SUB in tosa_001000_1
    {IR(57467, 6), 465}, // TABLE in tosa_001000_1
    {IR(57291, 5), 448}, // TANH in tosa_001000_1
    {IR(57694, 5), 494}, // TILE in tosa_001000_1
    {IR(57699, 10), 495}, // TRANSPOSE in tosa_001000_1
    {IR(57256, 17), 444}, // TRANSPOSE_CONV2D in tosa_001000_1
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
  "SPV_INTEL_subgroup_buffer_prefetch\0" // IR(3325, 35)
  "SPV_INTEL_subgroup_matrix_multiply_accumulate\0" // IR(3360, 46)
  "SPV_INTEL_subgroups\0" // IR(3406, 20)
  "SPV_INTEL_task_sequence\0" // IR(3426, 24)
  "SPV_INTEL_tensor_float32_conversion\0" // IR(3450, 36)
  "SPV_INTEL_ternary_bitwise_function\0" // IR(3486, 35)
  "SPV_INTEL_unstructured_loop_controls\0" // IR(3521, 37)
  "SPV_INTEL_usm_storage_classes\0" // IR(3558, 30)
  "SPV_INTEL_variable_length_array\0" // IR(3588, 32)
  "SPV_INTEL_vector_compute\0" // IR(3620, 25)
  "SPV_KHR_16bit_storage\0" // IR(3645, 22)
  "SPV_KHR_8bit_storage\0" // IR(3667, 21)
  "SPV_KHR_abort\0" // IR(3688, 14)
  "SPV_KHR_bfloat16\0" // IR(3702, 17)
  "SPV_KHR_bit_instructions\0" // IR(3719, 25)
  "SPV_KHR_compute_shader_derivatives\0" // IR(3744, 35)
  "SPV_KHR_constant_data\0" // IR(3779, 22)
  "SPV_KHR_cooperative_matrix\0" // IR(3801, 27)
  "SPV_KHR_device_group\0" // IR(3828, 21)
  "SPV_KHR_expect_assume\0" // IR(3849, 22)
  "SPV_KHR_float_controls\0" // IR(3871, 23)
  "SPV_KHR_float_controls2\0" // IR(3894, 24)
  "SPV_KHR_fma\0" // IR(3918, 12)
  "SPV_KHR_fragment_shader_barycentric\0" // IR(3930, 36)
  "SPV_KHR_fragment_shading_rate\0" // IR(3966, 30)
  "SPV_KHR_integer_dot_product\0" // IR(3996, 28)
  "SPV_KHR_linkonce_odr\0" // IR(4024, 21)
  "SPV_KHR_maximal_reconvergence\0" // IR(4045, 30)
  "SPV_KHR_multiview\0" // IR(4075, 18)
  "SPV_KHR_no_integer_wrap_decoration\0" // IR(4093, 35)
  "SPV_KHR_non_semantic_info\0" // IR(4128, 26)
  "SPV_KHR_opacity_micromap\0" // IR(4154, 25)
  "SPV_KHR_physical_storage_buffer\0" // IR(4179, 32)
  "SPV_KHR_poison_freeze\0" // IR(4211, 22)
  "SPV_KHR_post_depth_coverage\0" // IR(4233, 28)
  "SPV_KHR_quad_control\0" // IR(4261, 21)
  "SPV_KHR_ray_cull_mask\0" // IR(4282, 22)
  "SPV_KHR_ray_query\0" // IR(4304, 18)
  "SPV_KHR_ray_tracing\0" // IR(4322, 20)
  "SPV_KHR_ray_tracing_position_fetch\0" // IR(4342, 35)
  "SPV_KHR_relaxed_extended_instruction\0" // IR(4377, 37)
  "SPV_KHR_shader_atomic_counter_ops\0" // IR(4414, 34)
  "SPV_KHR_shader_ballot\0" // IR(4448, 22)
  "SPV_KHR_shader_clock\0" // IR(4470, 21)
  "SPV_KHR_shader_draw_parameters\0" // IR(4491, 31)
  "SPV_KHR_storage_buffer_storage_class\0" // IR(4522, 37)
  "SPV_KHR_subgroup_rotate\0" // IR(4559, 24)
  "SPV_KHR_subgroup_uniform_control_flow\0" // IR(4583, 38)
  "SPV_KHR_subgroup_vote\0" // IR(4621, 22)
  "SPV_KHR_terminate_invocation\0" // IR(4643, 29)
  "SPV_KHR_uniform_group_instructions\0" // IR(4672, 35)
  "SPV_KHR_untyped_pointers\0" // IR(4707, 25)
  "SPV_KHR_variable_pointers\0" // IR(4732, 26)
  "SPV_KHR_vulkan_memory_model\0" // IR(4758, 28)
  "SPV_KHR_workgroup_memory_explicit_layout\0" // IR(4786, 41)
  "SPV_NVX_multiview_per_view_attributes\0" // IR(4827, 38)
  "SPV_NV_bindless_texture\0" // IR(4865, 24)
  "SPV_NV_cluster_acceleration_structure\0" // IR(4889, 38)
  "SPV_NV_compute_shader_derivatives\0" // IR(4927, 34)
  "SPV_NV_cooperative_matrix\0" // IR(4961, 26)
  "SPV_NV_cooperative_matrix2\0" // IR(4987, 27)
  "SPV_NV_cooperative_matrix_decode_vector\0" // IR(5014, 40)
  "SPV_NV_cooperative_vector\0" // IR(5054, 26)
  "SPV_NV_displacement_micromap\0" // IR(5080, 29)
  "SPV_NV_fragment_shader_barycentric\0" // IR(5109, 35)
  "SPV_NV_geometry_shader_passthrough\0" // IR(5144, 35)
  "SPV_NV_linear_swept_spheres\0" // IR(5179, 28)
  "SPV_NV_mesh_shader\0" // IR(5207, 19)
  "SPV_NV_push_constant_bank\0" // IR(5226, 26)
  "SPV_NV_raw_access_chains\0" // IR(5252, 25)
  "SPV_NV_ray_tracing\0" // IR(5277, 19)
  "SPV_NV_ray_tracing_motion_blur\0" // IR(5296, 31)
  "SPV_NV_sample_mask_override_coverage\0" // IR(5327, 37)
  "SPV_NV_shader_atomic_fp16_vector\0" // IR(5364, 33)
  "SPV_NV_shader_image_footprint\0" // IR(5397, 30)
  "SPV_NV_shader_invocation_reorder\0" // IR(5427, 33)
  "SPV_NV_shader_sm_builtins\0" // IR(5460, 26)
  "SPV_NV_shader_subgroup_partitioned\0" // IR(5486, 35)
  "SPV_NV_shading_rate\0" // IR(5521, 20)
  "SPV_NV_stereo_view_rendering\0" // IR(5541, 29)
  "SPV_NV_tensor_addressing\0" // IR(5570, 25)
  "SPV_NV_viewport_array2\0" // IR(5595, 23)
  "SPV_QCOM_bfloat16_muladd\0" // IR(5618, 25)
  "SPV_QCOM_cooperative_matrix_conversion\0" // IR(5643, 39)
  "SPV_QCOM_image_processing\0" // IR(5682, 26)
  "SPV_QCOM_image_processing2\0" // IR(5708, 27)
  "SPV_QCOM_image_processing3\0" // IR(5735, 27)
  "SPV_QCOM_multiple_wait_queues\0" // IR(5762, 30)
  "SPV_QCOM_subgroup_size\0" // IR(5792, 23)
  "SPV_QCOM_tile_shading\0" // IR(5815, 22)
  "SPV_SAMSUNG_intrinsic\0" // IR(5837, 22)
  "SPV_VALIDATOR_ignore_type_decl_unique\0" // IR(5859, 38)
  "SPV_VALVE_mixed_float_dot_product\0" // IR(5897, 34)
  "ReadOnly\0" // IR(5931, 9)
  "Kernel\0" // IR(5940, 7)
  "WriteOnly\0" // IR(5947, 10)
  "ReadWrite\0" // IR(5957, 10)
  "Logical\0" // IR(5967, 8)
  "Physical32\0" // IR(5975, 11)
  "Addresses\0" // IR(5986, 10)
  "Physical64\0" // IR(5996, 11)
  "PhysicalStorageBuffer64\0" // IR(6007, 24)
  "PhysicalStorageBuffer64EXT\0" // IR(6031, 27)
  "PhysicalStorageBufferAddresses\0" // IR(6058, 31)
  "Position\0" // IR(6089, 9)
  "Shader\0" // IR(6098, 7)
  "PointSize\0" // IR(6105, 10)
  "ClipDistance\0" // IR(6115, 13)
  "CullDistance\0" // IR(6128, 13)
  "VertexId\0" // IR(6141, 9)
  "InstanceId\0" // IR(6150, 11)
  "PrimitiveId\0" // IR(6161, 12)
  "Geometry\0" // IR(6173, 9)
  "Tessellation\0" // IR(6182, 13)
  "RayTracingNV\0" // IR(6195, 13)
  "RayTracingKHR\0" // IR(6208, 14)
  "MeshShadingNV\0" // IR(6222, 14)
  "MeshShadingEXT\0" // IR(6236, 15)
  "InvocationId\0" // IR(6251, 13)
  "Layer\0" // IR(6264, 6)
  "ShaderLayer\0" // IR(6270, 12)
  "ShaderViewportIndexLayerEXT\0" // IR(6282, 28)
  "ViewportIndex\0" // IR(6310, 14)
  "MultiViewport\0" // IR(6324, 14)
  "ShaderViewportIndex\0" // IR(6338, 20)
  "TessLevelOuter\0" // IR(6358, 15)
  "TessLevelInner\0" // IR(6373, 15)
  "TessCoord\0" // IR(6388, 10)
  "PatchVertices\0" // IR(6398, 14)
  "FragCoord\0" // IR(6412, 10)
  "PointCoord\0" // IR(6422, 11)
  "FrontFacing\0" // IR(6433, 12)
  "SampleId\0" // IR(6445, 9)
  "SampleRateShading\0" // IR(6454, 18)
  "SamplePosition\0" // IR(6472, 15)
  "SampleMask\0" // IR(6487, 11)
  "FragDepth\0" // IR(6498, 10)
  "HelperInvocation\0" // IR(6508, 17)
  "NumWorkgroups\0" // IR(6525, 14)
  "WorkgroupSize\0" // IR(6539, 14)
  "WorkgroupId\0" // IR(6553, 12)
  "LocalInvocationId\0" // IR(6565, 18)
  "GlobalInvocationId\0" // IR(6583, 19)
  "LocalInvocationIndex\0" // IR(6602, 21)
  "WorkDim\0" // IR(6623, 8)
  "GlobalSize\0" // IR(6631, 11)
  "EnqueuedWorkgroupSize\0" // IR(6642, 22)
  "GlobalOffset\0" // IR(6664, 13)
  "GlobalLinearId\0" // IR(6677, 15)
  "SubgroupSize\0" // IR(6692, 13)
  "GroupNonUniform\0" // IR(6705, 16)
  "SubgroupBallotKHR\0" // IR(6721, 18)
  "SubgroupMaxSize\0" // IR(6739, 16)
  "NumSubgroups\0" // IR(6755, 13)
  "NumEnqueuedSubgroups\0" // IR(6768, 21)
  "SubgroupId\0" // IR(6789, 11)
  "SubgroupLocalInvocationId\0" // IR(6800, 26)
  "VertexIndex\0" // IR(6826, 12)
  "InstanceIndex\0" // IR(6838, 14)
  "CoreIDARM\0" // IR(6852, 10)
  "CoreBuiltinsARM\0" // IR(6862, 16)
  "CoreCountARM\0" // IR(6878, 13)
  "CoreMaxIDARM\0" // IR(6891, 13)
  "WarpIDARM\0" // IR(6904, 10)
  "WarpMaxIDARM\0" // IR(6914, 13)
  "SubgroupEqMask\0" // IR(6927, 15)
  "SubgroupEqMaskKHR\0" // IR(6942, 18)
  "GroupNonUniformBallot\0" // IR(6960, 22)
  "SubgroupGeMask\0" // IR(6982, 15)
  "SubgroupGeMaskKHR\0" // IR(6997, 18)
  "SubgroupGtMask\0" // IR(7015, 15)
  "SubgroupGtMaskKHR\0" // IR(7030, 18)
  "SubgroupLeMask\0" // IR(7048, 15)
  "SubgroupLeMaskKHR\0" // IR(7063, 18)
  "SubgroupLtMask\0" // IR(7081, 15)
  "SubgroupLtMaskKHR\0" // IR(7096, 18)
  "BaseVertex\0" // IR(7114, 11)
  "DrawParameters\0" // IR(7125, 15)
  "BaseInstance\0" // IR(7140, 13)
  "DrawIndex\0" // IR(7153, 10)
  "PrimitiveShadingRateKHR\0" // IR(7163, 24)
  "FragmentShadingRateKHR\0" // IR(7187, 23)
  "DeviceIndex\0" // IR(7210, 12)
  "DeviceGroup\0" // IR(7222, 12)
  "ViewIndex\0" // IR(7234, 10)
  "MultiView\0" // IR(7244, 10)
  "ShadingRateKHR\0" // IR(7254, 15)
  "TileOffsetQCOM\0" // IR(7269, 15)
  "TileShadingQCOM\0" // IR(7284, 16)
  "TileDimensionQCOM\0" // IR(7300, 18)
  "TileApronSizeQCOM\0" // IR(7318, 18)
  "BaryCoordNoPerspAMD\0" // IR(7336, 20)
  "BaryCoordNoPerspCentroidAMD\0" // IR(7356, 28)
  "BaryCoordNoPerspSampleAMD\0" // IR(7384, 26)
  "BaryCoordSmoothAMD\0" // IR(7410, 19)
  "BaryCoordSmoothCentroidAMD\0" // IR(7429, 27)
  "BaryCoordSmoothSampleAMD\0" // IR(7456, 25)
  "BaryCoordPullModelAMD\0" // IR(7481, 22)
  "FragStencilRefEXT\0" // IR(7503, 18)
  "StencilExportEXT\0" // IR(7521, 17)
  "RemainingRecursionLevelsAMDX\0" // IR(7538, 29)
  "ShaderEnqueueAMDX\0" // IR(7567, 18)
  "ShaderIndexAMDX\0" // IR(7585, 16)
  "SamplerHeapEXT\0" // IR(7601, 15)
  "DescriptorHeapEXT\0" // IR(7616, 18)
  "ResourceHeapEXT\0" // IR(7634, 16)
  "ViewportMaskNV\0" // IR(7650, 15)
  "ShaderViewportMaskNV\0" // IR(7665, 21)
  "SecondaryPositionNV\0" // IR(7686, 20)
  "ShaderStereoViewNV\0" // IR(7706, 19)
  "SecondaryViewportMaskNV\0" // IR(7725, 24)
  "PositionPerViewNV\0" // IR(7749, 18)
  "PerViewAttributesNV\0" // IR(7767, 20)
  "ViewportMaskPerViewNV\0" // IR(7787, 22)
  "FullyCoveredEXT\0" // IR(7809, 16)
  "FragmentFullyCoveredEXT\0" // IR(7825, 24)
  "TaskCountNV\0" // IR(7849, 12)
  "PrimitiveCountNV\0" // IR(7861, 17)
  "PrimitiveIndicesNV\0" // IR(7878, 19)
  "ClipDistancePerViewNV\0" // IR(7897, 22)
  "CullDistancePerViewNV\0" // IR(7919, 22)
  "LayerPerViewNV\0" // IR(7941, 15)
  "MeshViewCountNV\0" // IR(7956, 16)
  "MeshViewIndicesNV\0" // IR(7972, 18)
  "BaryCoordKHR\0" // IR(7990, 13)
  "BaryCoordNV\0" // IR(8003, 12)
  "FragmentBarycentricKHR\0" // IR(8015, 23)
  "BaryCoordNoPerspKHR\0" // IR(8038, 20)
  "BaryCoordNoPerspNV\0" // IR(8058, 19)
  "FragSizeEXT\0" // IR(8077, 12)
  "FragmentSizeNV\0" // IR(8089, 15)
  "FragmentDensityEXT\0" // IR(8104, 19)
  "FragInvocationCountEXT\0" // IR(8123, 23)
  "InvocationsPerPixelNV\0" // IR(8146, 22)
  "PrimitivePointIndicesEXT\0" // IR(8168, 25)
  "PrimitiveLineIndicesEXT\0" // IR(8193, 24)
  "PrimitiveTriangleIndicesEXT\0" // IR(8217, 28)
  "CullPrimitiveEXT\0" // IR(8245, 17)
  "LaunchIdKHR\0" // IR(8262, 12)
  "LaunchIdNV\0" // IR(8274, 11)
  "LaunchSizeKHR\0" // IR(8285, 14)
  "LaunchSizeNV\0" // IR(8299, 13)
  "WorldRayOriginKHR\0" // IR(8312, 18)
  "WorldRayOriginNV\0" // IR(8330, 17)
  "WorldRayDirectionKHR\0" // IR(8347, 21)
  "WorldRayDirectionNV\0" // IR(8368, 20)
  "ObjectRayOriginKHR\0" // IR(8388, 19)
  "ObjectRayOriginNV\0" // IR(8407, 18)
  "ObjectRayDirectionKHR\0" // IR(8425, 22)
  "ObjectRayDirectionNV\0" // IR(8447, 21)
  "RayTminKHR\0" // IR(8468, 11)
  "RayTminNV\0" // IR(8479, 10)
  "RayTmaxKHR\0" // IR(8489, 11)
  "RayTmaxNV\0" // IR(8500, 10)
  "InstanceCustomIndexKHR\0" // IR(8510, 23)
  "InstanceCustomIndexNV\0" // IR(8533, 22)
  "ObjectToWorldKHR\0" // IR(8555, 17)
  "ObjectToWorldNV\0" // IR(8572, 16)
  "WorldToObjectKHR\0" // IR(8588, 17)
  "WorldToObjectNV\0" // IR(8605, 16)
  "HitTNV\0" // IR(8621, 7)
  "HitKindKHR\0" // IR(8628, 11)
  "HitKindNV\0" // IR(8639, 10)
  "CurrentRayTimeNV\0" // IR(8649, 17)
  "RayTracingMotionBlurNV\0" // IR(8666, 23)
  "HitTriangleVertexPositionsKHR\0" // IR(8689, 30)
  "RayTracingPositionFetchKHR\0" // IR(8719, 27)
  "HitMicroTriangleVertexPositionsNV\0" // IR(8746, 34)
  "RayTracingDisplacementMicromapNV\0" // IR(8780, 33)
  "HitMicroTriangleVertexBarycentricsNV\0" // IR(8813, 37)
  "IncomingRayFlagsKHR\0" // IR(8850, 20)
  "IncomingRayFlagsNV\0" // IR(8870, 19)
  "RayGeometryIndexKHR\0" // IR(8889, 20)
  "HitIsSphereNV\0" // IR(8909, 14)
  "RayTracingSpheresGeometryNV\0" // IR(8923, 28)
  "HitIsLSSNV\0" // IR(8951, 11)
  "RayTracingLinearSweptSpheresGeometryNV\0" // IR(8962, 39)
  "HitSpherePositionNV\0" // IR(9001, 20)
  "WarpsPerSMNV\0" // IR(9021, 13)
  "ShaderSMBuiltinsNV\0" // IR(9034, 19)
  "SMCountNV\0" // IR(9053, 10)
  "WarpIDNV\0" // IR(9063, 9)
  "SMIDNV\0" // IR(9072, 7)
  "HitLSSPositionsNV\0" // IR(9079, 18)
  "HitKindFrontFacingMicroTriangleNV\0" // IR(9097, 34)
  "HitKindBackFacingMicroTriangleNV\0" // IR(9131, 33)
  "HitSphereRadiusNV\0" // IR(9164, 18)
  "HitLSSRadiiNV\0" // IR(9182, 14)
  "ClusterIDNV\0" // IR(9196, 12)
  "RayTracingClusterAccelerationStructureNV\0" // IR(9208, 41)
  "CullMaskKHR\0" // IR(9249, 12)
  "RayCullMaskKHR\0" // IR(9261, 15)
  "DeviceBarrierValidINTEL\0" // IR(9276, 24)
  "DeviceBarrierINTEL\0" // IR(9300, 19)
  "Matrix\0" // IR(9319, 7)
  "Linkage\0" // IR(9326, 8)
  "Vector16\0" // IR(9334, 9)
  "Float16Buffer\0" // IR(9343, 14)
  "Float16\0" // IR(9357, 8)
  "Float64\0" // IR(9365, 8)
  "Int64\0" // IR(9373, 6)
  "Int64Atomics\0" // IR(9379, 13)
  "ImageBasic\0" // IR(9392, 11)
  "ImageReadWrite\0" // IR(9403, 15)
  "ImageMipmap\0" // IR(9418, 12)
  "Pipes\0" // IR(9430, 6)
  "Groups\0" // IR(9436, 7)
  "DeviceEnqueue\0" // IR(9443, 14)
  "LiteralSampler\0" // IR(9457, 15)
  "AtomicStorage\0" // IR(9472, 14)
  "Int16\0" // IR(9486, 6)
  "TessellationPointSize\0" // IR(9492, 22)
  "GeometryPointSize\0" // IR(9514, 18)
  "ImageGatherExtended\0" // IR(9532, 20)
  "StorageImageMultisample\0" // IR(9552, 24)
  "UniformBufferArrayDynamicIndexing\0" // IR(9576, 34)
  "SampledImageArrayDynamicIndexing\0" // IR(9610, 33)
  "StorageBufferArrayDynamicIndexing\0" // IR(9643, 34)
  "StorageImageArrayDynamicIndexing\0" // IR(9677, 33)
  "ImageCubeArray\0" // IR(9710, 15)
  "SampledCubeArray\0" // IR(9725, 17)
  "ImageRect\0" // IR(9742, 10)
  "SampledRect\0" // IR(9752, 12)
  "GenericPointer\0" // IR(9764, 15)
  "Int8\0" // IR(9779, 5)
  "InputAttachment\0" // IR(9784, 16)
  "SparseResidency\0" // IR(9800, 16)
  "MinLod\0" // IR(9816, 7)
  "Sampled1D\0" // IR(9823, 10)
  "Image1D\0" // IR(9833, 8)
  "SampledBuffer\0" // IR(9841, 14)
  "ImageBuffer\0" // IR(9855, 12)
  "ImageMSArray\0" // IR(9867, 13)
  "StorageImageExtendedFormats\0" // IR(9880, 28)
  "ImageQuery\0" // IR(9908, 11)
  "DerivativeControl\0" // IR(9919, 18)
  "InterpolationFunction\0" // IR(9937, 22)
  "TransformFeedback\0" // IR(9959, 18)
  "GeometryStreams\0" // IR(9977, 16)
  "StorageImageReadWithoutFormat\0" // IR(9993, 30)
  "StorageImageWriteWithoutFormat\0" // IR(10023, 31)
  "SubgroupDispatch\0" // IR(10054, 17)
  "NamedBarrier\0" // IR(10071, 13)
  "PipeStorage\0" // IR(10084, 12)
  "GroupNonUniformVote\0" // IR(10096, 20)
  "GroupNonUniformArithmetic\0" // IR(10116, 26)
  "GroupNonUniformShuffle\0" // IR(10142, 23)
  "GroupNonUniformShuffleRelative\0" // IR(10165, 31)
  "GroupNonUniformClustered\0" // IR(10196, 25)
  "GroupNonUniformQuad\0" // IR(10221, 20)
  "UniformDecoration\0" // IR(10241, 18)
  "TileImageColorReadAccessEXT\0" // IR(10259, 28)
  "TileImageDepthReadAccessEXT\0" // IR(10287, 28)
  "TileImageStencilReadAccessEXT\0" // IR(10315, 30)
  "TensorsARM\0" // IR(10345, 11)
  "StorageTensorArrayDynamicIndexingARM\0" // IR(10356, 37)
  "StorageTensorArrayNonUniformIndexingARM\0" // IR(10393, 40)
  "GraphARM\0" // IR(10433, 9)
  "CooperativeMatrixLayoutsARM\0" // IR(10442, 28)
  "Float8EXT\0" // IR(10470, 10)
  "Float8CooperativeMatrixEXT\0" // IR(10480, 27)
  "CooperativeMatrixKHR\0" // IR(10507, 21)
  "Float6EXT\0" // IR(10528, 10)
  "Float4EXT\0" // IR(10538, 10)
  "Float8UnsignedE8M0EXT\0" // IR(10548, 22)
  "MXInt8EXT\0" // IR(10570, 10)
  "BitcastExtractEXT\0" // IR(10580, 18)
  "WorkgroupMemoryExplicitLayoutKHR\0" // IR(10598, 33)
  "WorkgroupMemoryExplicitLayout8BitAccessKHR\0" // IR(10631, 43)
  "WorkgroupMemoryExplicitLayout16BitAccessKHR\0" // IR(10674, 44)
  "SubgroupVoteKHR\0" // IR(10718, 16)
  "StorageBuffer16BitAccess\0" // IR(10734, 25)
  "StorageUniformBufferBlock16\0" // IR(10759, 28)
  "UniformAndStorageBuffer16BitAccess\0" // IR(10787, 35)
  "StorageUniform16\0" // IR(10822, 17)
  "StoragePushConstant16\0" // IR(10839, 22)
  "StorageInputOutput16\0" // IR(10861, 21)
  "VariablePointersStorageBuffer\0" // IR(10882, 30)
  "VariablePointers\0" // IR(10912, 17)
  "AtomicStorageOps\0" // IR(10929, 17)
  "SampleMaskPostDepthCoverage\0" // IR(10946, 28)
  "StorageBuffer8BitAccess\0" // IR(10974, 24)
  "UniformAndStorageBuffer8BitAccess\0" // IR(10998, 34)
  "StoragePushConstant8\0" // IR(11032, 21)
  "DenormPreserve\0" // IR(11053, 15)
  "DenormFlushToZero\0" // IR(11068, 18)
  "SignedZeroInfNanPreserve\0" // IR(11086, 25)
  "RoundingModeRTE\0" // IR(11111, 16)
  "RoundingModeRTZ\0" // IR(11127, 16)
  "RayQueryProvisionalKHR\0" // IR(11143, 23)
  "RayQueryKHR\0" // IR(11166, 12)
  "UntypedPointersKHR\0" // IR(11178, 19)
  "RayTraversalPrimitiveCullingKHR\0" // IR(11197, 32)
  "TextureSampleWeightedQCOM\0" // IR(11229, 26)
  "TextureBoxFilterQCOM\0" // IR(11255, 21)
  "TextureBlockMatchQCOM\0" // IR(11276, 22)
  "CooperativeMatrixConversionQCOM\0" // IR(11298, 32)
  "TextureBlockMatch2QCOM\0" // IR(11330, 23)
  "BFloat16MulAddQCOM\0" // IR(11353, 19)
  "SubgroupSizeQCOM\0" // IR(11372, 17)
  "MultipleWaitQueuesQCOM\0" // IR(11389, 23)
  "ImageGatherLinearQCOM\0" // IR(11412, 22)
  "ImageGatherExtendedModesQCOM\0" // IR(11434, 29)
  "Float16ImageAMD\0" // IR(11463, 16)
  "ImageGatherBiasLodAMD\0" // IR(11479, 22)
  "FragmentMaskAMD\0" // IR(11501, 16)
  "ImageReadWriteLodAMD\0" // IR(11517, 21)
  "Int64ImageEXT\0" // IR(11538, 14)
  "ShaderClockKHR\0" // IR(11552, 15)
  "QuadControlKHR\0" // IR(11567, 15)
  "Int4TypeINTEL\0" // IR(11582, 14)
  "Int4CooperativeMatrixINTEL\0" // IR(11596, 27)
  "BFloat16TypeKHR\0" // IR(11623, 16)
  "BFloat16DotProductKHR\0" // IR(11639, 22)
  "BFloat16CooperativeMatrixKHR\0" // IR(11661, 29)
  "AbortKHR\0" // IR(11690, 9)
  "ConstantDataKHR\0" // IR(11699, 16)
  "PoisonFreezeKHR\0" // IR(11715, 16)
  "WeakLinkageAMD\0" // IR(11731, 15)
  "SampleMaskOverrideCoverageNV\0" // IR(11746, 29)
  "GeometryShaderPassthroughNV\0" // IR(11775, 28)
  "ShaderViewportIndexLayerNV\0" // IR(11803, 27)
  "ImageFootprintNV\0" // IR(11830, 17)
  "FragmentBarycentricNV\0" // IR(11847, 22)
  "ComputeDerivativeGroupQuadsKHR\0" // IR(11869, 31)
  "ComputeDerivativeGroupQuadsNV\0" // IR(11900, 30)
  "ShadingRateNV\0" // IR(11930, 14)
  "GroupNonUniformPartitionedEXT\0" // IR(11944, 30)
  "GroupNonUniformPartitionedNV\0" // IR(11974, 29)
  "ShaderNonUniform\0" // IR(12003, 17)
  "ShaderNonUniformEXT\0" // IR(12020, 20)
  "RuntimeDescriptorArray\0" // IR(12040, 23)
  "RuntimeDescriptorArrayEXT\0" // IR(12063, 26)
  "InputAttachmentArrayDynamicIndexing\0" // IR(12089, 36)
  "InputAttachmentArrayDynamicIndexingEXT\0" // IR(12125, 39)
  "UniformTexelBufferArrayDynamicIndexing\0" // IR(12164, 39)
  "UniformTexelBufferArrayDynamicIndexingEXT\0" // IR(12203, 42)
  "StorageTexelBufferArrayDynamicIndexing\0" // IR(12245, 39)
  "StorageTexelBufferArrayDynamicIndexingEXT\0" // IR(12284, 42)
  "UniformBufferArrayNonUniformIndexing\0" // IR(12326, 37)
  "UniformBufferArrayNonUniformIndexingEXT\0" // IR(12363, 40)
  "SampledImageArrayNonUniformIndexing\0" // IR(12403, 36)
  "SampledImageArrayNonUniformIndexingEXT\0" // IR(12439, 39)
  "StorageBufferArrayNonUniformIndexing\0" // IR(12478, 37)
  "StorageBufferArrayNonUniformIndexingEXT\0" // IR(12515, 40)
  "StorageImageArrayNonUniformIndexing\0" // IR(12555, 36)
  "StorageImageArrayNonUniformIndexingEXT\0" // IR(12591, 39)
  "InputAttachmentArrayNonUniformIndexing\0" // IR(12630, 39)
  "InputAttachmentArrayNonUniformIndexingEXT\0" // IR(12669, 42)
  "UniformTexelBufferArrayNonUniformIndexing\0" // IR(12711, 42)
  "UniformTexelBufferArrayNonUniformIndexingEXT\0" // IR(12753, 45)
  "StorageTexelBufferArrayNonUniformIndexing\0" // IR(12798, 42)
  "StorageTexelBufferArrayNonUniformIndexingEXT\0" // IR(12840, 45)
  "VulkanMemoryModel\0" // IR(12885, 18)
  "VulkanMemoryModelKHR\0" // IR(12903, 21)
  "VulkanMemoryModelDeviceScope\0" // IR(12924, 29)
  "VulkanMemoryModelDeviceScopeKHR\0" // IR(12953, 32)
  "PhysicalStorageBufferAddressesEXT\0" // IR(12985, 34)
  "ComputeDerivativeGroupLinearKHR\0" // IR(13019, 32)
  "ComputeDerivativeGroupLinearNV\0" // IR(13051, 31)
  "RayTracingProvisionalKHR\0" // IR(13082, 25)
  "CooperativeMatrixNV\0" // IR(13107, 20)
  "FragmentShaderSampleInterlockEXT\0" // IR(13127, 33)
  "FragmentShaderShadingRateInterlockEXT\0" // IR(13160, 38)
  "FragmentShaderPixelInterlockEXT\0" // IR(13198, 32)
  "DemoteToHelperInvocation\0" // IR(13230, 25)
  "DemoteToHelperInvocationEXT\0" // IR(13255, 28)
  "DisplacementMicromapNV\0" // IR(13283, 23)
  "RayTracingOpacityMicromapKHR\0" // IR(13306, 29)
  "RayTracingOpacityMicromapEXT\0" // IR(13335, 29)
  "ShaderInvocationReorderNV\0" // IR(13364, 26)
  "ShaderInvocationReorderEXT\0" // IR(13390, 27)
  "BindlessTextureNV\0" // IR(13417, 18)
  "RayQueryPositionFetchKHR\0" // IR(13435, 25)
  "CooperativeVectorNV\0" // IR(13460, 20)
  "AtomicFloat16VectorNV\0" // IR(13480, 22)
  "RawAccessChainsNV\0" // IR(13502, 18)
  "PushConstantBanksNV\0" // IR(13520, 20)
  "LongVectorEXT\0" // IR(13540, 14)
  "Shader64BitIndexingEXT\0" // IR(13554, 23)
  "CooperativeMatrixConversionsEXT\0" // IR(13577, 32)
  "CooperativeMatrixReductionsEXT\0" // IR(13609, 31)
  "CooperativeMatrixReductionsNV\0" // IR(13640, 30)
  "CooperativeMatrixConversionsNV\0" // IR(13670, 31)
  "CooperativeMatrixPerElementOperationsEXT\0" // IR(13701, 41)
  "CooperativeMatrixPerElementOperationsNV\0" // IR(13742, 40)
  "CooperativeMatrixTensorAddressingNV\0" // IR(13782, 36)
  "CooperativeMatrixBlockLoadsNV\0" // IR(13818, 30)
  "CooperativeVectorTrainingNV\0" // IR(13848, 28)
  "CooperativeMatrixGetCoordinateEXT\0" // IR(13876, 34)
  "TensorAddressingNV\0" // IR(13910, 19)
  "CooperativeMatrixDecodeVectorNV\0" // IR(13929, 32)
  "SubgroupShuffleINTEL\0" // IR(13961, 21)
  "SubgroupBufferBlockIOINTEL\0" // IR(13982, 27)
  "SubgroupImageBlockIOINTEL\0" // IR(14009, 26)
  "SubgroupImageMediaBlockIOINTEL\0" // IR(14035, 31)
  "RoundToInfinityINTEL\0" // IR(14066, 21)
  "FloatingPointModeINTEL\0" // IR(14087, 23)
  "IntegerFunctions2INTEL\0" // IR(14110, 23)
  "FunctionPointersINTEL\0" // IR(14133, 22)
  "IndirectReferencesINTEL\0" // IR(14155, 24)
  "AsmINTEL\0" // IR(14179, 9)
  "AtomicFloat32MinMaxEXT\0" // IR(14188, 23)
  "AtomicFloat64MinMaxEXT\0" // IR(14211, 23)
  "AtomicFloat16MinMaxEXT\0" // IR(14234, 23)
  "VectorComputeINTEL\0" // IR(14257, 19)
  "VectorAnyINTEL\0" // IR(14276, 15)
  "ExpectAssumeKHR\0" // IR(14291, 16)
  "SubgroupAvcMotionEstimationINTEL\0" // IR(14307, 33)
  "SubgroupAvcMotionEstimationIntraINTEL\0" // IR(14340, 38)
  "SubgroupAvcMotionEstimationChromaINTEL\0" // IR(14378, 39)
  "VariableLengthArrayINTEL\0" // IR(14417, 25)
  "FunctionFloatControlINTEL\0" // IR(14442, 26)
  "FPGAMemoryAttributesALTERA\0" // IR(14468, 27)
  "FPGAMemoryAttributesINTEL\0" // IR(14495, 26)
  "FPFastMathModeINTEL\0" // IR(14521, 20)
  "ArbitraryPrecisionIntegersALTERA\0" // IR(14541, 33)
  "ArbitraryPrecisionIntegersINTEL\0" // IR(14574, 32)
  "ArbitraryPrecisionFloatingPointALTERA\0" // IR(14606, 38)
  "ArbitraryPrecisionFloatingPointINTEL\0" // IR(14644, 37)
  "UnstructuredLoopControlsINTEL\0" // IR(14681, 30)
  "FPGALoopControlsALTERA\0" // IR(14711, 23)
  "FPGALoopControlsINTEL\0" // IR(14734, 22)
  "KernelAttributesINTEL\0" // IR(14756, 22)
  "FPGAKernelAttributesINTEL\0" // IR(14778, 26)
  "FPGAMemoryAccessesALTERA\0" // IR(14804, 25)
  "FPGAMemoryAccessesINTEL\0" // IR(14829, 24)
  "FPGAClusterAttributesALTERA\0" // IR(14853, 28)
  "FPGAClusterAttributesINTEL\0" // IR(14881, 27)
  "LoopFuseALTERA\0" // IR(14908, 15)
  "LoopFuseINTEL\0" // IR(14923, 14)
  "FPGADSPControlALTERA\0" // IR(14937, 21)
  "FPGADSPControlINTEL\0" // IR(14958, 20)
  "MemoryAccessAliasingINTEL\0" // IR(14978, 26)
  "FPGAInvocationPipeliningAttributesALTERA\0" // IR(15004, 41)
  "FPGAInvocationPipeliningAttributesINTEL\0" // IR(15045, 40)
  "FPGABufferLocationALTERA\0" // IR(15085, 25)
  "FPGABufferLocationINTEL\0" // IR(15110, 24)
  "ArbitraryPrecisionFixedPointALTERA\0" // IR(15134, 35)
  "ArbitraryPrecisionFixedPointINTEL\0" // IR(15169, 34)
  "USMStorageClassesALTERA\0" // IR(15203, 24)
  "USMStorageClassesINTEL\0" // IR(15227, 23)
  "RuntimeAlignedAttributeALTERA\0" // IR(15250, 30)
  "RuntimeAlignedAttributeINTEL\0" // IR(15280, 29)
  "IOPipesALTERA\0" // IR(15309, 14)
  "IOPipesINTEL\0" // IR(15323, 13)
  "BlockingPipesALTERA\0" // IR(15336, 20)
  "BlockingPipesINTEL\0" // IR(15356, 19)
  "FPGARegALTERA\0" // IR(15375, 14)
  "FPGARegINTEL\0" // IR(15389, 13)
  "DotProductInputAll\0" // IR(15402, 19)
  "DotProductInputAllKHR\0" // IR(15421, 22)
  "DotProductInput4x8Bit\0" // IR(15443, 22)
  "DotProductInput4x8BitKHR\0" // IR(15465, 25)
  "DotProductInput4x8BitPacked\0" // IR(15490, 28)
  "DotProductInput4x8BitPackedKHR\0" // IR(15518, 31)
  "DotProduct\0" // IR(15549, 11)
  "DotProductKHR\0" // IR(15560, 14)
  "ReplicatedCompositesEXT\0" // IR(15574, 24)
  "BitInstructions\0" // IR(15598, 16)
  "GroupNonUniformRotateKHR\0" // IR(15614, 25)
  "FloatControls2\0" // IR(15639, 15)
  "FMAKHR\0" // IR(15654, 7)
  "RayTracingOpacityMicromapExecutionModeKHR\0" // IR(15661, 42)
  "AtomicFloat32AddEXT\0" // IR(15703, 20)
  "AtomicFloat64AddEXT\0" // IR(15723, 20)
  "LongCompositesINTEL\0" // IR(15743, 20)
  "OptNoneEXT\0" // IR(15763, 11)
  "OptNoneINTEL\0" // IR(15774, 13)
  "AtomicFloat16AddEXT\0" // IR(15787, 20)
  "DebugInfoModuleINTEL\0" // IR(15807, 21)
  "BFloat16ConversionINTEL\0" // IR(15828, 24)
  "SplitBarrierEXT\0" // IR(15852, 16)
  "SplitBarrierINTEL\0" // IR(15868, 18)
  "ArithmeticFenceEXT\0" // IR(15886, 19)
  "FPGAClusterAttributesV2ALTERA\0" // IR(15905, 30)
  "FPGAClusterAttributesV2INTEL\0" // IR(15935, 29)
  "FPGAKernelAttributesv2INTEL\0" // IR(15964, 28)
  "TaskSequenceALTERA\0" // IR(15992, 19)
  "TaskSequenceINTEL\0" // IR(16011, 18)
  "FPMaxErrorINTEL\0" // IR(16029, 16)
  "FPGALatencyControlALTERA\0" // IR(16045, 25)
  "FPGALatencyControlINTEL\0" // IR(16070, 24)
  "FPGAArgumentInterfacesALTERA\0" // IR(16094, 29)
  "FPGAArgumentInterfacesINTEL\0" // IR(16123, 28)
  "GlobalVariableHostAccessINTEL\0" // IR(16151, 30)
  "GlobalVariableFPGADecorationsALTERA\0" // IR(16181, 36)
  "GlobalVariableFPGADecorationsINTEL\0" // IR(16217, 35)
  "SubgroupBufferPrefetchINTEL\0" // IR(16252, 28)
  "Subgroup2DBlockIOINTEL\0" // IR(16280, 23)
  "Subgroup2DBlockTransformINTEL\0" // IR(16303, 30)
  "Subgroup2DBlockTransposeINTEL\0" // IR(16333, 30)
  "SubgroupMatrixMultiplyAccumulateINTEL\0" // IR(16363, 38)
  "TernaryBitwiseFunctionINTEL\0" // IR(16401, 28)
  "UntypedVariableLengthArrayINTEL\0" // IR(16429, 32)
  "SpecConditionalINTEL\0" // IR(16461, 21)
  "FunctionVariantsINTEL\0" // IR(16482, 22)
  "PredicatedIOINTEL\0" // IR(16504, 18)
  "RoundedDivideSqrtINTEL\0" // IR(16522, 23)
  "GroupUniformArithmeticKHR\0" // IR(16545, 26)
  "TensorFloat32RoundingINTEL\0" // IR(16571, 27)
  "MaskedGatherScatterINTEL\0" // IR(16598, 25)
  "CacheControlsINTEL\0" // IR(16623, 19)
  "RegisterLimitsINTEL\0" // IR(16642, 20)
  "BindlessImagesINTEL\0" // IR(16662, 20)
  "DotProductFloat16AccFloat32VALVE\0" // IR(16682, 33)
  "DotProductFloat16AccFloat16VALVE\0" // IR(16715, 33)
  "DotProductBFloat16AccVALVE\0" // IR(16748, 27)
  "DotProductFloat8AccFloat32VALVE\0" // IR(16775, 32)
  "IntrinsicSAMSUNG\0" // IR(16807, 17)
  "Unspecified\0" // IR(16824, 12)
  "Address\0" // IR(16836, 8)
  "Boolean\0" // IR(16844, 8)
  "Float\0" // IR(16852, 6)
  "Signed\0" // IR(16858, 7)
  "SignedChar\0" // IR(16865, 11)
  "Unsigned\0" // IR(16876, 9)
  "UnsignedChar\0" // IR(16885, 13)
  "Class\0" // IR(16898, 6)
  "Structure\0" // IR(16904, 10)
  "Union\0" // IR(16914, 6)
  "ImportedModule\0" // IR(16920, 15)
  "ImportedDeclaration\0" // IR(16935, 20)
  "None\0" // IR(16955, 5)
  "FlagIsProtected\0" // IR(16960, 16)
  "FlagIsPrivate\0" // IR(16976, 14)
  "FlagIsPublic\0" // IR(16990, 13)
  "FlagIsLocal\0" // IR(17003, 12)
  "FlagIsDefinition\0" // IR(17015, 17)
  "FlagFwdDecl\0" // IR(17032, 12)
  "FlagArtificial\0" // IR(17044, 15)
  "FlagExplicit\0" // IR(17059, 13)
  "FlagPrototyped\0" // IR(17072, 15)
  "FlagObjectPointer\0" // IR(17087, 18)
  "FlagStaticMember\0" // IR(17105, 17)
  "FlagIndirectVariable\0" // IR(17122, 21)
  "FlagLValueReference\0" // IR(17143, 20)
  "FlagRValueReference\0" // IR(17163, 20)
  "FlagIsOptimized\0" // IR(17183, 16)
  "FlagIsEnumClass\0" // IR(17199, 16)
  "FlagTypePassByValue\0" // IR(17215, 20)
  "FlagTypePassByReference\0" // IR(17235, 24)
  "Deref\0" // IR(17259, 6)
  "Plus\0" // IR(17265, 5)
  "Minus\0" // IR(17270, 6)
  "SPV_OPERAND_TYPE_LITERAL_INTEGER\0" // IR(17276, 33)
  "PlusUconst\0" // IR(17309, 11)
  "BitPiece\0" // IR(17320, 9)
  "Swap\0" // IR(17329, 5)
  "Xderef\0" // IR(17334, 7)
  "StackValue\0" // IR(17341, 11)
  "Constu\0" // IR(17352, 7)
  "Fragment\0" // IR(17359, 9)
  "ConstType\0" // IR(17368, 10)
  "VolatileType\0" // IR(17378, 13)
  "RestrictType\0" // IR(17391, 13)
  "AtomicType\0" // IR(17404, 11)
  "Float16NV\0" // IR(17415, 10)
  "Float32NV\0" // IR(17425, 10)
  "Float64NV\0" // IR(17435, 10)
  "SignedInt8NV\0" // IR(17445, 13)
  "SignedInt16NV\0" // IR(17458, 14)
  "SignedInt32NV\0" // IR(17472, 14)
  "SignedInt64NV\0" // IR(17486, 14)
  "UnsignedInt8NV\0" // IR(17500, 15)
  "UnsignedInt16NV\0" // IR(17515, 16)
  "UnsignedInt32NV\0" // IR(17531, 16)
  "UnsignedInt64NV\0" // IR(17547, 16)
  "SignedInt8PackedNV\0" // IR(17563, 19)
  "UnsignedInt8PackedNV\0" // IR(17582, 21)
  "FloatE4M3NV\0" // IR(17603, 12)
  "FloatE5M2NV\0" // IR(17615, 12)
  "RowMajorKHR\0" // IR(17627, 12)
  "ColumnMajorKHR\0" // IR(17639, 15)
  "RowBlockedInterleavedARM\0" // IR(17654, 25)
  "ColumnBlockedInterleavedARM\0" // IR(17679, 28)
  "NoneKHR\0" // IR(17707, 8)
  "MatrixASignedComponentsKHR\0" // IR(17715, 27)
  "MatrixBSignedComponentsKHR\0" // IR(17742, 27)
  "MatrixCSignedComponentsKHR\0" // IR(17769, 27)
  "MatrixResultSignedComponentsKHR\0" // IR(17796, 32)
  "SaturatingAccumulationKHR\0" // IR(17828, 26)
  "Row\0" // IR(17854, 4)
  "Column\0" // IR(17858, 7)
  "2x2\0" // IR(17865, 4)
  "MatrixAKHR\0" // IR(17869, 11)
  "MatrixBKHR\0" // IR(17880, 11)
  "MatrixAccumulatorKHR\0" // IR(17891, 21)
  "RowMajorNV\0" // IR(17912, 11)
  "ColumnMajorNV\0" // IR(17923, 14)
  "InferencingOptimalNV\0" // IR(17937, 21)
  "TrainingOptimalNV\0" // IR(17958, 18)
  "RelaxedPrecision\0" // IR(17976, 17)
  "SpecId\0" // IR(17993, 7)
  "Block\0" // IR(18000, 6)
  "BufferBlock\0" // IR(18006, 12)
  "RowMajor\0" // IR(18018, 9)
  "ColMajor\0" // IR(18027, 9)
  "ArrayStride\0" // IR(18036, 12)
  "MatrixStride\0" // IR(18048, 13)
  "GLSLShared\0" // IR(18061, 11)
  "GLSLPacked\0" // IR(18072, 11)
  "CPacked\0" // IR(18083, 8)
  "SPV_OPERAND_TYPE_BUILT_IN\0" // IR(18091, 26)
  "BuiltIn\0" // IR(18117, 8)
  "NoPerspective\0" // IR(18125, 14)
  "Flat\0" // IR(18139, 5)
  "Patch\0" // IR(18144, 6)
  "Centroid\0" // IR(18150, 9)
  "Sample\0" // IR(18159, 7)
  "Invariant\0" // IR(18166, 10)
  "Restrict\0" // IR(18176, 9)
  "Aliased\0" // IR(18185, 8)
  "Volatile\0" // IR(18193, 9)
  "Constant\0" // IR(18202, 9)
  "Coherent\0" // IR(18211, 9)
  "NonWritable\0" // IR(18220, 12)
  "NonReadable\0" // IR(18232, 12)
  "Uniform\0" // IR(18244, 8)
  "SPV_OPERAND_TYPE_SCOPE_ID\0" // IR(18252, 26)
  "UniformId\0" // IR(18278, 10)
  "SaturatedConversion\0" // IR(18288, 20)
  "Stream\0" // IR(18308, 7)
  "Location\0" // IR(18315, 9)
  "Component\0" // IR(18324, 10)
  "Index\0" // IR(18334, 6)
  "Binding\0" // IR(18340, 8)
  "DescriptorSet\0" // IR(18348, 14)
  "Offset\0" // IR(18362, 7)
  "XfbBuffer\0" // IR(18369, 10)
  "XfbStride\0" // IR(18379, 10)
  "SPV_OPERAND_TYPE_FUNCTION_PARAMETER_ATTRIBUTE\0" // IR(18389, 46)
  "FuncParamAttr\0" // IR(18435, 14)
  "SPV_OPERAND_TYPE_FP_ROUNDING_MODE\0" // IR(18449, 34)
  "FPRoundingMode\0" // IR(18483, 15)
  "SPV_OPERAND_TYPE_FP_FAST_MATH_MODE\0" // IR(18498, 35)
  "FPFastMathMode\0" // IR(18533, 15)
  "SPV_OPERAND_TYPE_LITERAL_STRING\0" // IR(18548, 32)
  "SPV_OPERAND_TYPE_LINKAGE_TYPE\0" // IR(18580, 30)
  "LinkageAttributes\0" // IR(18610, 18)
  "NoContraction\0" // IR(18628, 14)
  "InputAttachmentIndex\0" // IR(18642, 21)
  "Alignment\0" // IR(18663, 10)
  "MaxByteOffset\0" // IR(18673, 14)
  "SPV_OPERAND_TYPE_ID\0" // IR(18687, 20)
  "AlignmentId\0" // IR(18707, 12)
  "MaxByteOffsetId\0" // IR(18719, 16)
  "SaturatedToLargestFloat8NormalConversionEXT\0" // IR(18735, 44)
  "NoSignedWrap\0" // IR(18779, 13)
  "NoUnsignedWrap\0" // IR(18792, 15)
  "WeightTextureQCOM\0" // IR(18807, 18)
  "BlockMatchTextureQCOM\0" // IR(18825, 22)
  "BlockMatchSamplerQCOM\0" // IR(18847, 22)
  "ExplicitInterpAMD\0" // IR(18869, 18)
  "NodeSharesPayloadLimitsWithAMDX\0" // IR(18887, 32)
  "NodeMaxPayloadsAMDX\0" // IR(18919, 20)
  "TrackFinishWritingAMDX\0" // IR(18939, 23)
  "PayloadNodeNameAMDX\0" // IR(18962, 20)
  "PayloadNodeBaseIndexAMDX\0" // IR(18982, 25)
  "PayloadNodeSparseArrayAMDX\0" // IR(19007, 27)
  "PayloadNodeArraySizeAMDX\0" // IR(19034, 25)
  "PayloadDispatchIndirectAMDX\0" // IR(19059, 28)
  "ArrayStrideIdEXT\0" // IR(19087, 17)
  "OffsetIdEXT\0" // IR(19104, 12)
  "UTFEncodedKHR\0" // IR(19116, 14)
  "OverrideCoverageNV\0" // IR(19130, 19)
  "PassthroughNV\0" // IR(19149, 14)
  "ViewportRelativeNV\0" // IR(19163, 19)
  "SecondaryViewportRelativeNV\0" // IR(19182, 28)
  "PerPrimitiveEXT\0" // IR(19210, 16)
  "PerPrimitiveNV\0" // IR(19226, 15)
  "PerViewNV\0" // IR(19241, 10)
  "PerTaskNV\0" // IR(19251, 10)
  "PerVertexKHR\0" // IR(19261, 13)
  "PerVertexNV\0" // IR(19274, 12)
  "NonUniform\0" // IR(19286, 11)
  "NonUniformEXT\0" // IR(19297, 14)
  "RestrictPointer\0" // IR(19311, 16)
  "RestrictPointerEXT\0" // IR(19327, 19)
  "AliasedPointer\0" // IR(19346, 15)
  "AliasedPointerEXT\0" // IR(19361, 18)
  "MemberOffsetNV\0" // IR(19379, 15)
  "HitObjectShaderRecordBufferNV\0" // IR(19394, 30)
  "HitObjectShaderRecordBufferEXT\0" // IR(19424, 31)
  "BankNV\0" // IR(19455, 7)
  "BindlessSamplerNV\0" // IR(19462, 18)
  "BindlessImageNV\0" // IR(19480, 16)
  "BoundSamplerNV\0" // IR(19496, 15)
  "BoundImageNV\0" // IR(19511, 13)
  "CooperativeMatrixTransposeEXT\0" // IR(19524, 30)
  "SIMTCallINTEL\0" // IR(19554, 14)
  "ReferencedIndirectlyINTEL\0" // IR(19568, 26)
  "ClobberINTEL\0" // IR(19594, 13)
  "SideEffectsINTEL\0" // IR(19607, 17)
  "VectorComputeVariableINTEL\0" // IR(19624, 27)
  "FuncParamIOKindINTEL\0" // IR(19651, 21)
  "VectorComputeFunctionINTEL\0" // IR(19672, 27)
  "StackCallINTEL\0" // IR(19699, 15)
  "GlobalVariableOffsetINTEL\0" // IR(19714, 26)
  "CounterBuffer\0" // IR(19740, 14)
  "HlslCounterBufferGOOGLE\0" // IR(19754, 24)
  "UserSemantic\0" // IR(19778, 13)
  "HlslSemanticGOOGLE\0" // IR(19791, 19)
  "UserTypeGOOGLE\0" // IR(19810, 15)
  "FunctionRoundingModeINTEL\0" // IR(19825, 26)
  "SPV_OPERAND_TYPE_FPDENORM_MODE\0" // IR(19851, 31)
  "FunctionDenormModeINTEL\0" // IR(19882, 24)
  "RegisterALTERA\0" // IR(19906, 15)
  "RegisterINTEL\0" // IR(19921, 14)
  "MemoryALTERA\0" // IR(19935, 13)
  "MemoryINTEL\0" // IR(19948, 12)
  "NumbanksALTERA\0" // IR(19960, 15)
  "NumbanksINTEL\0" // IR(19975, 14)
  "BankwidthALTERA\0" // IR(19989, 16)
  "BankwidthINTEL\0" // IR(20005, 15)
  "MaxPrivateCopiesALTERA\0" // IR(20020, 23)
  "MaxPrivateCopiesINTEL\0" // IR(20043, 22)
  "SinglepumpALTERA\0" // IR(20065, 17)
  "SinglepumpINTEL\0" // IR(20082, 16)
  "DoublepumpALTERA\0" // IR(20098, 17)
  "DoublepumpINTEL\0" // IR(20115, 16)
  "MaxReplicatesALTERA\0" // IR(20131, 20)
  "MaxReplicatesINTEL\0" // IR(20151, 19)
  "SimpleDualPortALTERA\0" // IR(20170, 21)
  "SimpleDualPortINTEL\0" // IR(20191, 20)
  "MergeALTERA\0" // IR(20211, 12)
  "MergeINTEL\0" // IR(20223, 11)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER\0" // IR(20234, 42)
  "BankBitsALTERA\0" // IR(20276, 15)
  "BankBitsINTEL\0" // IR(20291, 14)
  "ForcePow2DepthALTERA\0" // IR(20305, 21)
  "ForcePow2DepthINTEL\0" // IR(20326, 20)
  "StridesizeALTERA\0" // IR(20346, 17)
  "StridesizeINTEL\0" // IR(20363, 16)
  "WordsizeALTERA\0" // IR(20379, 15)
  "WordsizeINTEL\0" // IR(20394, 14)
  "TrueDualPortALTERA\0" // IR(20408, 19)
  "TrueDualPortINTEL\0" // IR(20427, 18)
  "BurstCoalesceALTERA\0" // IR(20445, 20)
  "BurstCoalesceINTEL\0" // IR(20465, 19)
  "CacheSizeALTERA\0" // IR(20484, 16)
  "CacheSizeINTEL\0" // IR(20500, 15)
  "DontStaticallyCoalesceALTERA\0" // IR(20515, 29)
  "DontStaticallyCoalesceINTEL\0" // IR(20544, 28)
  "PrefetchALTERA\0" // IR(20572, 15)
  "PrefetchINTEL\0" // IR(20587, 14)
  "StallEnableALTERA\0" // IR(20601, 18)
  "StallEnableINTEL\0" // IR(20619, 17)
  "FuseLoopsInFunctionALTERA\0" // IR(20636, 26)
  "FuseLoopsInFunctionINTEL\0" // IR(20662, 25)
  "MathOpDSPModeALTERA\0" // IR(20687, 20)
  "MathOpDSPModeINTEL\0" // IR(20707, 19)
  "AliasScopeINTEL\0" // IR(20726, 16)
  "NoAliasINTEL\0" // IR(20742, 13)
  "InitiationIntervalALTERA\0" // IR(20755, 25)
  "InitiationIntervalINTEL\0" // IR(20780, 24)
  "MaxConcurrencyALTERA\0" // IR(20804, 21)
  "MaxConcurrencyINTEL\0" // IR(20825, 20)
  "PipelineEnableALTERA\0" // IR(20845, 21)
  "PipelineEnableINTEL\0" // IR(20866, 20)
  "BufferLocationALTERA\0" // IR(20886, 21)
  "BufferLocationINTEL\0" // IR(20907, 20)
  "IOPipeStorageALTERA\0" // IR(20927, 20)
  "IOPipeStorageINTEL\0" // IR(20947, 19)
  "SPV_OPERAND_TYPE_FPOPERATION_MODE\0" // IR(20966, 34)
  "FunctionFloatingPointModeINTEL\0" // IR(21000, 31)
  "SingleElementVectorINTEL\0" // IR(21031, 25)
  "VectorComputeCallableFunctionINTEL\0" // IR(21056, 35)
  "MediaBlockIOINTEL\0" // IR(21091, 18)
  "StallFreeALTERA\0" // IR(21109, 16)
  "StallFreeINTEL\0" // IR(21125, 15)
  "SPV_OPERAND_TYPE_LITERAL_FLOAT\0" // IR(21140, 31)
  "FPMaxErrorDecorationINTEL\0" // IR(21171, 26)
  "LatencyControlLabelALTERA\0" // IR(21197, 26)
  "LatencyControlLabelINTEL\0" // IR(21223, 25)
  "LatencyControlConstraintALTERA\0" // IR(21248, 31)
  "LatencyControlConstraintINTEL\0" // IR(21279, 30)
  "ConduitKernelArgumentALTERA\0" // IR(21309, 28)
  "ConduitKernelArgumentINTEL\0" // IR(21337, 27)
  "RegisterMapKernelArgumentALTERA\0" // IR(21364, 32)
  "RegisterMapKernelArgumentINTEL\0" // IR(21396, 31)
  "MMHostInterfaceAddressWidthALTERA\0" // IR(21427, 34)
  "MMHostInterfaceAddressWidthINTEL\0" // IR(21461, 33)
  "MMHostInterfaceDataWidthALTERA\0" // IR(21494, 31)
  "MMHostInterfaceDataWidthINTEL\0" // IR(21525, 30)
  "MMHostInterfaceLatencyALTERA\0" // IR(21555, 29)
  "MMHostInterfaceLatencyINTEL\0" // IR(21584, 28)
  "SPV_OPERAND_TYPE_ACCESS_QUALIFIER\0" // IR(21612, 34)
  "MMHostInterfaceReadWriteModeALTERA\0" // IR(21646, 35)
  "MMHostInterfaceReadWriteModeINTEL\0" // IR(21681, 34)
  "MMHostInterfaceMaxBurstALTERA\0" // IR(21715, 30)
  "MMHostInterfaceMaxBurstINTEL\0" // IR(21745, 29)
  "MMHostInterfaceWaitRequestALTERA\0" // IR(21774, 33)
  "MMHostInterfaceWaitRequestINTEL\0" // IR(21807, 32)
  "StableKernelArgumentALTERA\0" // IR(21839, 27)
  "StableKernelArgumentINTEL\0" // IR(21866, 26)
  "SPV_OPERAND_TYPE_HOST_ACCESS_QUALIFIER\0" // IR(21892, 39)
  "HostAccessINTEL\0" // IR(21931, 16)
  "SPV_OPERAND_TYPE_INITIALIZATION_MODE_QUALIFIER\0" // IR(21947, 47)
  "InitModeALTERA\0" // IR(21994, 15)
  "InitModeINTEL\0" // IR(22009, 14)
  "ImplementInRegisterMapALTERA\0" // IR(22023, 29)
  "ImplementInRegisterMapINTEL\0" // IR(22052, 28)
  "ConditionalINTEL\0" // IR(22080, 17)
  "SPV_OPERAND_TYPE_LOAD_CACHE_CONTROL\0" // IR(22097, 36)
  "CacheControlLoadINTEL\0" // IR(22133, 22)
  "SPV_OPERAND_TYPE_STORE_CACHE_CONTROL\0" // IR(22155, 37)
  "CacheControlStoreINTEL\0" // IR(22192, 23)
  "1D\0" // IR(22215, 3)
  "2D\0" // IR(22218, 3)
  "3D\0" // IR(22221, 3)
  "Cube\0" // IR(22224, 5)
  "Rect\0" // IR(22229, 5)
  "Buffer\0" // IR(22234, 7)
  "SubpassData\0" // IR(22241, 12)
  "TileImageDataEXT\0" // IR(22253, 17)
  "Invocations\0" // IR(22270, 12)
  "SpacingEqual\0" // IR(22282, 13)
  "SpacingFractionalEven\0" // IR(22295, 22)
  "SpacingFractionalOdd\0" // IR(22317, 21)
  "VertexOrderCw\0" // IR(22338, 14)
  "VertexOrderCcw\0" // IR(22352, 15)
  "PixelCenterInteger\0" // IR(22367, 19)
  "OriginUpperLeft\0" // IR(22386, 16)
  "OriginLowerLeft\0" // IR(22402, 16)
  "EarlyFragmentTests\0" // IR(22418, 19)
  "PointMode\0" // IR(22437, 10)
  "Xfb\0" // IR(22447, 4)
  "DepthReplacing\0" // IR(22451, 15)
  "DepthGreater\0" // IR(22466, 13)
  "DepthLess\0" // IR(22479, 10)
  "DepthUnchanged\0" // IR(22489, 15)
  "LocalSize\0" // IR(22504, 10)
  "LocalSizeHint\0" // IR(22514, 14)
  "InputPoints\0" // IR(22528, 12)
  "InputLines\0" // IR(22540, 11)
  "InputLinesAdjacency\0" // IR(22551, 20)
  "Triangles\0" // IR(22571, 10)
  "InputTrianglesAdjacency\0" // IR(22581, 24)
  "Quads\0" // IR(22605, 6)
  "Isolines\0" // IR(22611, 9)
  "OutputVertices\0" // IR(22620, 15)
  "OutputPoints\0" // IR(22635, 13)
  "OutputLineStrip\0" // IR(22648, 16)
  "OutputTriangleStrip\0" // IR(22664, 20)
  "VecTypeHint\0" // IR(22684, 12)
  "ContractionOff\0" // IR(22696, 15)
  "Initializer\0" // IR(22711, 12)
  "Finalizer\0" // IR(22723, 10)
  "SubgroupsPerWorkgroup\0" // IR(22733, 22)
  "SubgroupsPerWorkgroupId\0" // IR(22755, 24)
  "LocalSizeId\0" // IR(22779, 12)
  "LocalSizeHintId\0" // IR(22791, 16)
  "NonCoherentColorAttachmentReadEXT\0" // IR(22807, 34)
  "NonCoherentDepthAttachmentReadEXT\0" // IR(22841, 34)
  "NonCoherentStencilAttachmentReadEXT\0" // IR(22875, 36)
  "SubgroupUniformControlFlowKHR\0" // IR(22911, 30)
  "PostDepthCoverage\0" // IR(22941, 18)
  "NonCoherentTileAttachmentReadQCOM\0" // IR(22959, 34)
  "TileShadingRateQCOM\0" // IR(22993, 20)
  "SubgroupSizeHalfQCOM\0" // IR(23013, 21)
  "SubgroupSizeFullQCOM\0" // IR(23034, 21)
  "EarlyAndLateFragmentTestsAMD\0" // IR(23055, 29)
  "StencilRefReplacingEXT\0" // IR(23084, 23)
  "CoalescingAMDX\0" // IR(23107, 15)
  "IsApiEntryAMDX\0" // IR(23122, 15)
  "MaxNodeRecursionAMDX\0" // IR(23137, 21)
  "StaticNumWorkgroupsAMDX\0" // IR(23158, 24)
  "MaxNumWorkgroupsAMDX\0" // IR(23182, 21)
  "StencilRefUnchangedFrontAMD\0" // IR(23203, 28)
  "StencilRefGreaterFrontAMD\0" // IR(23231, 26)
  "StencilRefLessFrontAMD\0" // IR(23257, 23)
  "StencilRefUnchangedBackAMD\0" // IR(23280, 27)
  "StencilRefGreaterBackAMD\0" // IR(23307, 25)
  "StencilRefLessBackAMD\0" // IR(23332, 22)
  "QuadDerivativesKHR\0" // IR(23354, 19)
  "RequireFullQuadsKHR\0" // IR(23373, 20)
  "SharesInputWithAMDX\0" // IR(23393, 20)
  "ArithmeticPoisonKHR\0" // IR(23413, 20)
  "OutputLinesEXT\0" // IR(23433, 15)
  "OutputLinesNV\0" // IR(23448, 14)
  "OutputPrimitivesEXT\0" // IR(23462, 20)
  "OutputPrimitivesNV\0" // IR(23482, 19)
  "DerivativeGroupQuadsKHR\0" // IR(23501, 24)
  "DerivativeGroupQuadsNV\0" // IR(23525, 23)
  "DerivativeGroupLinearKHR\0" // IR(23548, 25)
  "DerivativeGroupLinearNV\0" // IR(23573, 24)
  "OutputTrianglesEXT\0" // IR(23597, 19)
  "OutputTrianglesNV\0" // IR(23616, 18)
  "PixelInterlockOrderedEXT\0" // IR(23634, 25)
  "PixelInterlockUnorderedEXT\0" // IR(23659, 27)
  "SampleInterlockOrderedEXT\0" // IR(23686, 26)
  "SampleInterlockUnorderedEXT\0" // IR(23712, 28)
  "ShadingRateInterlockOrderedEXT\0" // IR(23740, 31)
  "ShadingRateInterlockUnorderedEXT\0" // IR(23771, 33)
  "SharedLocalMemorySizeINTEL\0" // IR(23804, 27)
  "RoundingModeRTPINTEL\0" // IR(23831, 21)
  "RoundingModeRTNINTEL\0" // IR(23852, 21)
  "FloatingPointModeALTINTEL\0" // IR(23873, 26)
  "FloatingPointModeIEEEINTEL\0" // IR(23899, 27)
  "MaxWorkgroupSizeINTEL\0" // IR(23926, 22)
  "MaxWorkDimINTEL\0" // IR(23948, 16)
  "NoGlobalOffsetINTEL\0" // IR(23964, 20)
  "NumSIMDWorkitemsINTEL\0" // IR(23984, 22)
  "SchedulerTargetFmaxMhzINTEL\0" // IR(24006, 28)
  "MaximallyReconvergesKHR\0" // IR(24034, 24)
  "FPFastMathDefault\0" // IR(24058, 18)
  "OpacityMicromapIdKHR\0" // IR(24076, 21)
  "StreamingInterfaceINTEL\0" // IR(24097, 24)
  "RegisterMapInterfaceINTEL\0" // IR(24121, 26)
  "NamedBarrierCountINTEL\0" // IR(24147, 23)
  "MaximumRegistersINTEL\0" // IR(24170, 22)
  "MaximumRegistersIdINTEL\0" // IR(24192, 24)
  "SPV_OPERAND_TYPE_NAMED_MAXIMUM_NUMBER_OF_REGISTERS\0" // IR(24216, 51)
  "NamedMaximumRegistersINTEL\0" // IR(24267, 27)
  "Vertex\0" // IR(24294, 7)
  "TessellationControl\0" // IR(24301, 20)
  "TessellationEvaluation\0" // IR(24321, 23)
  "GLCompute\0" // IR(24344, 10)
  "TaskNV\0" // IR(24354, 7)
  "MeshNV\0" // IR(24361, 7)
  "RayGenerationKHR\0" // IR(24368, 17)
  "RayGenerationNV\0" // IR(24385, 16)
  "IntersectionKHR\0" // IR(24401, 16)
  "IntersectionNV\0" // IR(24417, 15)
  "AnyHitKHR\0" // IR(24432, 10)
  "AnyHitNV\0" // IR(24442, 9)
  "ClosestHitKHR\0" // IR(24451, 14)
  "ClosestHitNV\0" // IR(24465, 13)
  "MissKHR\0" // IR(24478, 8)
  "MissNV\0" // IR(24486, 7)
  "CallableKHR\0" // IR(24493, 12)
  "CallableNV\0" // IR(24505, 11)
  "TaskEXT\0" // IR(24516, 8)
  "MeshEXT\0" // IR(24524, 8)
  "Preserve\0" // IR(24532, 9)
  "FlushToZero\0" // IR(24541, 12)
  "BFloat16KHR\0" // IR(24553, 12)
  "Float8E4M3EXT\0" // IR(24565, 14)
  "Float8E5M2EXT\0" // IR(24579, 14)
  "Float6E2M3EXT\0" // IR(24593, 14)
  "Float6E3M2EXT\0" // IR(24607, 14)
  "Float4E2M1EXT\0" // IR(24621, 14)
  "IEEE\0" // IR(24635, 5)
  "ALT\0" // IR(24640, 4)
  "NotNaN\0" // IR(24644, 7)
  "NotInf\0" // IR(24651, 7)
  "NSZ\0" // IR(24658, 4)
  "AllowRecip\0" // IR(24662, 11)
  "Fast\0" // IR(24673, 5)
  "AllowContract\0" // IR(24678, 14)
  "AllowContractFastINTEL\0" // IR(24692, 23)
  "AllowReassoc\0" // IR(24715, 13)
  "AllowReassocINTEL\0" // IR(24728, 18)
  "AllowTransform\0" // IR(24746, 15)
  "RTE\0" // IR(24761, 4)
  "RTZ\0" // IR(24765, 4)
  "RTP\0" // IR(24769, 4)
  "RTN\0" // IR(24773, 4)
  "Vertical2Pixels\0" // IR(24777, 16)
  "Vertical4Pixels\0" // IR(24793, 16)
  "Horizontal2Pixels\0" // IR(24809, 18)
  "Horizontal4Pixels\0" // IR(24827, 18)
  "Inline\0" // IR(24845, 7)
  "DontInline\0" // IR(24852, 11)
  "Pure\0" // IR(24863, 5)
  "Const\0" // IR(24868, 6)
  "Zext\0" // IR(24874, 5)
  "Sext\0" // IR(24879, 5)
  "ByVal\0" // IR(24884, 6)
  "Sret\0" // IR(24890, 5)
  "NoAlias\0" // IR(24895, 8)
  "NoCapture\0" // IR(24903, 10)
  "NoWrite\0" // IR(24913, 8)
  "NoReadWrite\0" // IR(24921, 12)
  "RuntimeAlignedALTERA\0" // IR(24933, 21)
  "RuntimeAlignedINTEL\0" // IR(24954, 20)
  "Gather4x1QCOM\0" // IR(24974, 14)
  "GatherDQCOM\0" // IR(24988, 12)
  "GatherH2QCOM\0" // IR(25000, 13)
  "GatherV2QCOM\0" // IR(25013, 13)
  "Reduce\0" // IR(25026, 7)
  "InclusiveScan\0" // IR(25033, 14)
  "ExclusiveScan\0" // IR(25047, 14)
  "ClusteredReduce\0" // IR(25061, 16)
  "PartitionedReduceEXT\0" // IR(25077, 21)
  "PartitionedReduceNV\0" // IR(25098, 20)
  "PartitionedInclusiveScanEXT\0" // IR(25118, 28)
  "PartitionedInclusiveScanNV\0" // IR(25146, 27)
  "PartitionedExclusiveScanEXT\0" // IR(25173, 28)
  "PartitionedExclusiveScanNV\0" // IR(25201, 27)
  "NoneINTEL\0" // IR(25228, 10)
  "ReadINTEL\0" // IR(25238, 10)
  "WriteINTEL\0" // IR(25248, 11)
  "ReadWriteINTEL\0" // IR(25259, 15)
  "Bias\0" // IR(25274, 5)
  "Lod\0" // IR(25279, 4)
  "Grad\0" // IR(25283, 5)
  "ConstOffset\0" // IR(25288, 12)
  "ConstOffsets\0" // IR(25300, 13)
  "MakeTexelAvailable\0" // IR(25313, 19)
  "MakeTexelAvailableKHR\0" // IR(25332, 22)
  "MakeTexelVisible\0" // IR(25354, 17)
  "MakeTexelVisibleKHR\0" // IR(25371, 20)
  "NonPrivateTexel\0" // IR(25391, 16)
  "NonPrivateTexelKHR\0" // IR(25407, 19)
  "VolatileTexel\0" // IR(25426, 14)
  "VolatileTexelKHR\0" // IR(25440, 17)
  "SignExtend\0" // IR(25457, 11)
  "ZeroExtend\0" // IR(25468, 11)
  "Nontemporal\0" // IR(25479, 12)
  "Offsets\0" // IR(25491, 8)
  "SnormInt8\0" // IR(25499, 10)
  "SnormInt16\0" // IR(25509, 11)
  "UnormInt8\0" // IR(25520, 10)
  "UnormInt16\0" // IR(25530, 11)
  "UnormShort565\0" // IR(25541, 14)
  "UnormShort555\0" // IR(25555, 14)
  "UnormInt101010\0" // IR(25569, 15)
  "SignedInt8\0" // IR(25584, 11)
  "SignedInt16\0" // IR(25595, 12)
  "SignedInt32\0" // IR(25607, 12)
  "UnsignedInt8\0" // IR(25619, 13)
  "UnsignedInt16\0" // IR(25632, 14)
  "UnsignedInt32\0" // IR(25646, 14)
  "HalfFloat\0" // IR(25660, 10)
  "UnormInt24\0" // IR(25670, 11)
  "UnormInt101010_2\0" // IR(25681, 17)
  "UnormInt10X6EXT\0" // IR(25698, 16)
  "UnsignedIntRaw10EXT\0" // IR(25714, 20)
  "UnsignedIntRaw12EXT\0" // IR(25734, 20)
  "UnormInt2_101010EXT\0" // IR(25754, 20)
  "UnsignedInt10X6EXT\0" // IR(25774, 19)
  "UnsignedInt12X4EXT\0" // IR(25793, 19)
  "UnsignedInt14X2EXT\0" // IR(25812, 19)
  "UnormInt12X4EXT\0" // IR(25831, 16)
  "UnormInt14X2EXT\0" // IR(25847, 16)
  "R\0" // IR(25863, 2)
  "A\0" // IR(25865, 2)
  "RG\0" // IR(25867, 3)
  "RA\0" // IR(25870, 3)
  "RGB\0" // IR(25873, 4)
  "RGBA\0" // IR(25877, 5)
  "BGRA\0" // IR(25882, 5)
  "ARGB\0" // IR(25887, 5)
  "Intensity\0" // IR(25892, 10)
  "Luminance\0" // IR(25902, 10)
  "Rx\0" // IR(25912, 3)
  "RGx\0" // IR(25915, 4)
  "RGBx\0" // IR(25919, 5)
  "Depth\0" // IR(25924, 6)
  "DepthStencil\0" // IR(25930, 13)
  "sRGB\0" // IR(25943, 5)
  "sRGBx\0" // IR(25948, 6)
  "sRGBA\0" // IR(25954, 6)
  "sBGRA\0" // IR(25960, 6)
  "ABGR\0" // IR(25966, 5)
  "InitOnDeviceReprogramALTERA\0" // IR(25971, 28)
  "InitOnDeviceReprogramINTEL\0" // IR(25999, 27)
  "InitOnDeviceResetALTERA\0" // IR(26026, 24)
  "InitOnDeviceResetINTEL\0" // IR(26050, 23)
  "NoWait\0" // IR(26073, 7)
  "WaitKernel\0" // IR(26080, 11)
  "WaitWorkGroup\0" // IR(26091, 14)
  "CmdExecTime\0" // IR(26105, 12)
  "MayUsePrintf\0" // IR(26117, 13)
  "Export\0" // IR(26130, 7)
  "Import\0" // IR(26137, 7)
  "LinkOnceODR\0" // IR(26144, 12)
  "WeakAMD\0" // IR(26156, 8)
  "UncachedINTEL\0" // IR(26164, 14)
  "CachedINTEL\0" // IR(26178, 12)
  "StreamingINTEL\0" // IR(26190, 15)
  "InvalidateAfterReadINTEL\0" // IR(26205, 25)
  "ConstCachedINTEL\0" // IR(26230, 17)
  "Unroll\0" // IR(26247, 7)
  "DontUnroll\0" // IR(26254, 11)
  "DependencyInfinite\0" // IR(26265, 19)
  "DependencyLength\0" // IR(26284, 17)
  "MinIterations\0" // IR(26301, 14)
  "MaxIterations\0" // IR(26315, 14)
  "IterationMultiple\0" // IR(26329, 18)
  "PeelCount\0" // IR(26347, 10)
  "PartialCount\0" // IR(26357, 13)
  "DependencyArrayALTERA\0" // IR(26370, 22)
  "DependencyArrayINTEL\0" // IR(26392, 21)
  "LoopCoalesceALTERA\0" // IR(26413, 19)
  "LoopCoalesceINTEL\0" // IR(26432, 18)
  "MaxInterleavingALTERA\0" // IR(26450, 22)
  "MaxInterleavingINTEL\0" // IR(26472, 21)
  "SpeculatedIterationsALTERA\0" // IR(26493, 27)
  "SpeculatedIterationsINTEL\0" // IR(26520, 26)
  "NoFusionALTERA\0" // IR(26546, 15)
  "NoFusionINTEL\0" // IR(26561, 14)
  "LoopCountALTERA\0" // IR(26575, 16)
  "LoopCountINTEL\0" // IR(26591, 15)
  "MaxReinvocationDelayALTERA\0" // IR(26606, 27)
  "MaxReinvocationDelayINTEL\0" // IR(26633, 26)
  "MatrixASignedComponentsINTEL\0" // IR(26659, 29)
  "MatrixBSignedComponentsINTEL\0" // IR(26688, 29)
  "MatrixCBFloat16INTEL\0" // IR(26717, 21)
  "MatrixResultBFloat16INTEL\0" // IR(26738, 26)
  "MatrixAPackedInt8INTEL\0" // IR(26764, 23)
  "MatrixBPackedInt8INTEL\0" // IR(26787, 23)
  "MatrixAPackedInt4INTEL\0" // IR(26810, 23)
  "MatrixBPackedInt4INTEL\0" // IR(26833, 23)
  "MatrixATF32INTEL\0" // IR(26856, 17)
  "MatrixBTF32INTEL\0" // IR(26873, 17)
  "MatrixAPackedFloat16INTEL\0" // IR(26890, 26)
  "MatrixBPackedFloat16INTEL\0" // IR(26916, 26)
  "MatrixAPackedBFloat16INTEL\0" // IR(26942, 27)
  "MatrixBPackedBFloat16INTEL\0" // IR(26969, 27)
  "Aligned\0" // IR(26996, 8)
  "MakePointerAvailable\0" // IR(27004, 21)
  "MakePointerAvailableKHR\0" // IR(27025, 24)
  "MakePointerVisible\0" // IR(27049, 19)
  "MakePointerVisibleKHR\0" // IR(27068, 22)
  "NonPrivatePointer\0" // IR(27090, 18)
  "NonPrivatePointerKHR\0" // IR(27108, 21)
  "AliasScopeINTELMask\0" // IR(27129, 20)
  "NoAliasINTELMask\0" // IR(27149, 17)
  "Simple\0" // IR(27166, 7)
  "GLSL450\0" // IR(27173, 8)
  "OpenCL\0" // IR(27181, 7)
  "Vulkan\0" // IR(27188, 7)
  "VulkanKHR\0" // IR(27195, 10)
  "Relaxed\0" // IR(27205, 8)
  "Acquire\0" // IR(27213, 8)
  "Release\0" // IR(27221, 8)
  "AcquireRelease\0" // IR(27229, 15)
  "SequentiallyConsistent\0" // IR(27244, 23)
  "UniformMemory\0" // IR(27267, 14)
  "SubgroupMemory\0" // IR(27281, 15)
  "WorkgroupMemory\0" // IR(27296, 16)
  "CrossWorkgroupMemory\0" // IR(27312, 21)
  "AtomicCounterMemory\0" // IR(27333, 20)
  "ImageMemory\0" // IR(27353, 12)
  "OutputMemory\0" // IR(27365, 13)
  "OutputMemoryKHR\0" // IR(27378, 16)
  "MakeAvailable\0" // IR(27394, 14)
  "MakeAvailableKHR\0" // IR(27408, 17)
  "MakeVisible\0" // IR(27425, 12)
  "MakeVisibleKHR\0" // IR(27437, 15)
  "AutoINTEL\0" // IR(27452, 10)
  "WRAP\0" // IR(27462, 5)
  "SAT\0" // IR(27467, 4)
  "SAT_ZERO\0" // IR(27471, 9)
  "SAT_SYM\0" // IR(27480, 8)
  "PackedVectorFormat4x8Bit\0" // IR(27488, 25)
  "PackedVectorFormat4x8BitKHR\0" // IR(27513, 28)
  "TRN\0" // IR(27541, 4)
  "TRN_ZERO\0" // IR(27545, 9)
  "RND\0" // IR(27554, 4)
  "RND_ZERO\0" // IR(27558, 9)
  "RND_INF\0" // IR(27567, 8)
  "RND_MIN_INF\0" // IR(27575, 12)
  "RND_CONV\0" // IR(27587, 9)
  "RND_CONV_ODD\0" // IR(27596, 13)
  "RobustnessPerComponentNV\0" // IR(27609, 25)
  "RobustnessPerElementNV\0" // IR(27634, 23)
  "OpaqueKHR\0" // IR(27657, 10)
  "NoOpaqueKHR\0" // IR(27667, 12)
  "TerminateOnFirstHitKHR\0" // IR(27679, 23)
  "SkipClosestHitShaderKHR\0" // IR(27702, 24)
  "CullBackFacingTrianglesKHR\0" // IR(27726, 27)
  "CullFrontFacingTrianglesKHR\0" // IR(27753, 28)
  "CullOpaqueKHR\0" // IR(27781, 14)
  "CullNoOpaqueKHR\0" // IR(27795, 16)
  "SkipTrianglesKHR\0" // IR(27811, 17)
  "SkipBuiltinPrimitivesNV\0" // IR(27828, 24)
  "SkipAABBsKHR\0" // IR(27852, 13)
  "ForceOpacityMicromap2StateKHR\0" // IR(27865, 30)
  "ForceOpacityMicromap2StateEXT\0" // IR(27895, 30)
  "RayQueryCandidateIntersectionTriangleKHR\0" // IR(27925, 41)
  "RayQueryCandidateIntersectionAABBKHR\0" // IR(27966, 37)
  "RayQueryCommittedIntersectionNoneKHR\0" // IR(28003, 37)
  "RayQueryCommittedIntersectionTriangleKHR\0" // IR(28040, 41)
  "RayQueryCommittedIntersectionGeneratedKHR\0" // IR(28081, 42)
  "RayQueryCandidateIntersectionKHR\0" // IR(28123, 33)
  "RayQueryCommittedIntersectionKHR\0" // IR(28156, 33)
  "ClampToEdge\0" // IR(28189, 12)
  "Clamp\0" // IR(28201, 6)
  "Repeat\0" // IR(28207, 7)
  "RepeatMirrored\0" // IR(28214, 15)
  "Nearest\0" // IR(28229, 8)
  "Linear\0" // IR(28237, 7)
  "Unknown\0" // IR(28244, 8)
  "Rgba32f\0" // IR(28252, 8)
  "Rgba16f\0" // IR(28260, 8)
  "R32f\0" // IR(28268, 5)
  "Rgba8\0" // IR(28273, 6)
  "Rgba8Snorm\0" // IR(28279, 11)
  "Rg32f\0" // IR(28290, 6)
  "Rg16f\0" // IR(28296, 6)
  "R11fG11fB10f\0" // IR(28302, 13)
  "R16f\0" // IR(28315, 5)
  "Rgba16\0" // IR(28320, 7)
  "Rgb10A2\0" // IR(28327, 8)
  "Rg16\0" // IR(28335, 5)
  "Rg8\0" // IR(28340, 4)
  "R16\0" // IR(28344, 4)
  "R8\0" // IR(28348, 3)
  "Rgba16Snorm\0" // IR(28351, 12)
  "Rg16Snorm\0" // IR(28363, 10)
  "Rg8Snorm\0" // IR(28373, 9)
  "R16Snorm\0" // IR(28382, 9)
  "R8Snorm\0" // IR(28391, 8)
  "Rgba32i\0" // IR(28399, 8)
  "Rgba16i\0" // IR(28407, 8)
  "Rgba8i\0" // IR(28415, 7)
  "R32i\0" // IR(28422, 5)
  "Rg32i\0" // IR(28427, 6)
  "Rg16i\0" // IR(28433, 6)
  "Rg8i\0" // IR(28439, 5)
  "R16i\0" // IR(28444, 5)
  "R8i\0" // IR(28449, 4)
  "Rgba32ui\0" // IR(28453, 9)
  "Rgba16ui\0" // IR(28462, 9)
  "Rgba8ui\0" // IR(28471, 8)
  "R32ui\0" // IR(28479, 6)
  "Rgb10a2ui\0" // IR(28485, 10)
  "Rg32ui\0" // IR(28495, 7)
  "Rg16ui\0" // IR(28502, 7)
  "Rg8ui\0" // IR(28509, 6)
  "R16ui\0" // IR(28515, 6)
  "R8ui\0" // IR(28521, 5)
  "R64ui\0" // IR(28526, 6)
  "R64i\0" // IR(28532, 5)
  "CrossDevice\0" // IR(28537, 12)
  "Device\0" // IR(28549, 7)
  "Workgroup\0" // IR(28556, 10)
  "Subgroup\0" // IR(28566, 9)
  "Invocation\0" // IR(28575, 11)
  "QueueFamily\0" // IR(28586, 12)
  "QueueFamilyKHR\0" // IR(28598, 15)
  "ShaderCallKHR\0" // IR(28613, 14)
  "Flatten\0" // IR(28627, 8)
  "DontFlatten\0" // IR(28635, 12)
  "IdentifierPossibleDuplicates\0" // IR(28647, 29)
  "FlagUnknownPhysicalLayout\0" // IR(28676, 26)
  "ESSL\0" // IR(28702, 5)
  "GLSL\0" // IR(28707, 5)
  "OpenCL_C\0" // IR(28712, 9)
  "OpenCL_CPP\0" // IR(28721, 11)
  "HLSL\0" // IR(28732, 5)
  "CPP_for_OpenCL\0" // IR(28737, 15)
  "SYCL\0" // IR(28752, 5)
  "HERO_C\0" // IR(28757, 7)
  "NZSL\0" // IR(28764, 5)
  "WGSL\0" // IR(28769, 5)
  "Slang\0" // IR(28774, 6)
  "Zig\0" // IR(28780, 4)
  "Rust\0" // IR(28784, 5)
  "Pred\0" // IR(28789, 5)
  "ApilaJai\0" // IR(28794, 9)
  "UniformConstant\0" // IR(28803, 16)
  "Input\0" // IR(28819, 6)
  "Output\0" // IR(28825, 7)
  "CrossWorkgroup\0" // IR(28832, 15)
  "Private\0" // IR(28847, 8)
  "Function\0" // IR(28855, 9)
  "Generic\0" // IR(28864, 8)
  "PushConstant\0" // IR(28872, 13)
  "AtomicCounter\0" // IR(28885, 14)
  "Image\0" // IR(28899, 6)
  "StorageBuffer\0" // IR(28905, 14)
  "TileImageEXT\0" // IR(28919, 13)
  "TileAttachmentQCOM\0" // IR(28932, 19)
  "NodePayloadAMDX\0" // IR(28951, 16)
  "CallableDataKHR\0" // IR(28967, 16)
  "CallableDataNV\0" // IR(28983, 15)
  "IncomingCallableDataKHR\0" // IR(28998, 24)
  "IncomingCallableDataNV\0" // IR(29022, 23)
  "RayPayloadKHR\0" // IR(29045, 14)
  "RayPayloadNV\0" // IR(29059, 13)
  "HitAttributeKHR\0" // IR(29072, 16)
  "HitAttributeNV\0" // IR(29088, 15)
  "IncomingRayPayloadKHR\0" // IR(29103, 22)
  "IncomingRayPayloadNV\0" // IR(29125, 21)
  "ShaderRecordBufferKHR\0" // IR(29146, 22)
  "ShaderRecordBufferNV\0" // IR(29168, 21)
  "PhysicalStorageBuffer\0" // IR(29189, 22)
  "PhysicalStorageBufferEXT\0" // IR(29211, 25)
  "HitObjectAttributeNV\0" // IR(29236, 21)
  "TaskPayloadWorkgroupEXT\0" // IR(29257, 24)
  "HitObjectAttributeEXT\0" // IR(29281, 22)
  "CodeSectionINTEL\0" // IR(29303, 17)
  "DeviceOnlyALTERA\0" // IR(29320, 17)
  "DeviceOnlyINTEL\0" // IR(29337, 16)
  "HostOnlyALTERA\0" // IR(29353, 15)
  "HostOnlyINTEL\0" // IR(29368, 14)
  "WriteThroughINTEL\0" // IR(29382, 18)
  "WriteBackINTEL\0" // IR(29400, 15)
  "TensorView\0" // IR(29415, 11)
  "DecodeFunc\0" // IR(29426, 11)
  "DecodeVectorFunc\0" // IR(29437, 17)
  "Undefined\0" // IR(29454, 10)
  "NoneARM\0" // IR(29464, 8)
  "NontemporalARM\0" // IR(29472, 15)
  "OutOfBoundsValueARM\0" // IR(29487, 20)
  "MakeElementAvailableARM\0" // IR(29507, 24)
  "MakeElementVisibleARM\0" // IR(29531, 22)
  "NonPrivateElementARM\0" // IR(29553, 21)
  "Nop\0" // IR(29574, 4)
  "SPV_OPERAND_TYPE_TYPE_ID\0" // IR(29578, 25)
  "SPV_OPERAND_TYPE_RESULT_ID\0" // IR(29603, 27)
  "Undef\0" // IR(29630, 6)
  "SourceContinued\0" // IR(29636, 16)
  "SPV_OPERAND_TYPE_SOURCE_LANGUAGE\0" // IR(29652, 33)
  "SPV_OPERAND_TYPE_OPTIONAL_ID\0" // IR(29685, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_STRING\0" // IR(29714, 41)
  "Source\0" // IR(29755, 7)
  "SourceExtension\0" // IR(29762, 16)
  "Name\0" // IR(29778, 5)
  "MemberName\0" // IR(29783, 11)
  "String\0" // IR(29794, 7)
  "Line\0" // IR(29801, 5)
  "Extension\0" // IR(29806, 10)
  "ExtInstImport\0" // IR(29816, 14)
  "SPV_OPERAND_TYPE_EXTENSION_INSTRUCTION_NUMBER\0" // IR(29830, 46)
  "ExtInst\0" // IR(29876, 8)
  "SPV_OPERAND_TYPE_ADDRESSING_MODEL\0" // IR(29884, 34)
  "SPV_OPERAND_TYPE_MEMORY_MODEL\0" // IR(29918, 30)
  "MemoryModel\0" // IR(29948, 12)
  "SPV_OPERAND_TYPE_EXECUTION_MODEL\0" // IR(29960, 33)
  "SPV_OPERAND_TYPE_VARIABLE_ID\0" // IR(29993, 29)
  "EntryPoint\0" // IR(30022, 11)
  "SPV_OPERAND_TYPE_EXECUTION_MODE\0" // IR(30033, 32)
  "ExecutionMode\0" // IR(30065, 14)
  "SPV_OPERAND_TYPE_CAPABILITY\0" // IR(30079, 28)
  "Capability\0" // IR(30107, 11)
  "TypeVoid\0" // IR(30118, 9)
  "TypeBool\0" // IR(30127, 9)
  "TypeInt\0" // IR(30136, 8)
  "SPV_OPERAND_TYPE_OPTIONAL_FPENCODING\0" // IR(30144, 37)
  "TypeFloat\0" // IR(30181, 10)
  "TypeVector\0" // IR(30191, 11)
  "TypeMatrix\0" // IR(30202, 11)
  "SPV_OPERAND_TYPE_DIMENSIONALITY\0" // IR(30213, 32)
  "SPV_OPERAND_TYPE_SAMPLER_IMAGE_FORMAT\0" // IR(30245, 38)
  "SPV_OPERAND_TYPE_OPTIONAL_ACCESS_QUALIFIER\0" // IR(30283, 43)
  "TypeImage\0" // IR(30326, 10)
  "TypeSampler\0" // IR(30336, 12)
  "TypeSampledImage\0" // IR(30348, 17)
  "TypeArray\0" // IR(30365, 10)
  "TypeRuntimeArray\0" // IR(30375, 17)
  "TypeStruct\0" // IR(30392, 11)
  "TypeOpaque\0" // IR(30403, 11)
  "SPV_OPERAND_TYPE_STORAGE_CLASS\0" // IR(30414, 31)
  "TypePointer\0" // IR(30445, 12)
  "TypeFunction\0" // IR(30457, 13)
  "TypeEvent\0" // IR(30470, 10)
  "TypeDeviceEvent\0" // IR(30480, 16)
  "TypeReserveId\0" // IR(30496, 14)
  "TypeQueue\0" // IR(30510, 10)
  "TypePipe\0" // IR(30520, 9)
  "TypeForwardPointer\0" // IR(30529, 19)
  "ConstantTrue\0" // IR(30548, 13)
  "ConstantFalse\0" // IR(30561, 14)
  "SPV_OPERAND_TYPE_TYPED_LITERAL_NUMBER\0" // IR(30575, 38)
  "ConstantComposite\0" // IR(30613, 18)
  "SPV_OPERAND_TYPE_SAMPLER_ADDRESSING_MODE\0" // IR(30631, 41)
  "SPV_OPERAND_TYPE_SAMPLER_FILTER_MODE\0" // IR(30672, 37)
  "ConstantSampler\0" // IR(30709, 16)
  "ConstantNull\0" // IR(30725, 13)
  "SpecConstantTrue\0" // IR(30738, 17)
  "SpecConstantFalse\0" // IR(30755, 18)
  "SpecConstant\0" // IR(30773, 13)
  "SpecConstantComposite\0" // IR(30786, 22)
  "SPV_OPERAND_TYPE_SPEC_CONSTANT_OP_NUMBER\0" // IR(30808, 41)
  "SpecConstantOp\0" // IR(30849, 15)
  "SPV_OPERAND_TYPE_FUNCTION_CONTROL\0" // IR(30864, 34)
  "FunctionParameter\0" // IR(30898, 18)
  "FunctionEnd\0" // IR(30916, 12)
  "FunctionCall\0" // IR(30928, 13)
  "Variable\0" // IR(30941, 9)
  "ImageTexelPointer\0" // IR(30950, 18)
  "SPV_OPERAND_TYPE_OPTIONAL_MEMORY_ACCESS\0" // IR(30968, 40)
  "Load\0" // IR(31008, 5)
  "Store\0" // IR(31013, 6)
  "CopyMemory\0" // IR(31019, 11)
  "CopyMemorySized\0" // IR(31030, 16)
  "AccessChain\0" // IR(31046, 12)
  "InBoundsAccessChain\0" // IR(31058, 20)
  "PtrAccessChain\0" // IR(31078, 15)
  "ArrayLength\0" // IR(31093, 12)
  "GenericPtrMemSemantics\0" // IR(31105, 23)
  "InBoundsPtrAccessChain\0" // IR(31128, 23)
  "SPV_OPERAND_TYPE_DECORATION\0" // IR(31151, 28)
  "Decorate\0" // IR(31179, 9)
  "MemberDecorate\0" // IR(31188, 15)
  "DecorationGroup\0" // IR(31203, 16)
  "GroupDecorate\0" // IR(31219, 14)
  "SPV_OPERAND_TYPE_VARIABLE_ID_LITERAL_INTEGER\0" // IR(31233, 45)
  "GroupMemberDecorate\0" // IR(31278, 20)
  "VectorExtractDynamic\0" // IR(31298, 21)
  "VectorInsertDynamic\0" // IR(31319, 20)
  "VectorShuffle\0" // IR(31339, 14)
  "CompositeConstruct\0" // IR(31353, 19)
  "CompositeExtract\0" // IR(31372, 17)
  "CompositeInsert\0" // IR(31389, 16)
  "CopyObject\0" // IR(31405, 11)
  "Transpose\0" // IR(31416, 10)
  "SampledImage\0" // IR(31426, 13)
  "SPV_OPERAND_TYPE_OPTIONAL_IMAGE\0" // IR(31439, 32)
  "ImageSampleImplicitLod\0" // IR(31471, 23)
  "SPV_OPERAND_TYPE_IMAGE\0" // IR(31494, 23)
  "ImageSampleExplicitLod\0" // IR(31517, 23)
  "ImageSampleDrefImplicitLod\0" // IR(31540, 27)
  "ImageSampleDrefExplicitLod\0" // IR(31567, 27)
  "ImageSampleProjImplicitLod\0" // IR(31594, 27)
  "ImageSampleProjExplicitLod\0" // IR(31621, 27)
  "ImageSampleProjDrefImplicitLod\0" // IR(31648, 31)
  "ImageSampleProjDrefExplicitLod\0" // IR(31679, 31)
  "ImageFetch\0" // IR(31710, 11)
  "ImageGather\0" // IR(31721, 12)
  "ImageDrefGather\0" // IR(31733, 16)
  "ImageRead\0" // IR(31749, 10)
  "ImageWrite\0" // IR(31759, 11)
  "ImageQueryFormat\0" // IR(31770, 17)
  "ImageQueryOrder\0" // IR(31787, 16)
  "ImageQuerySizeLod\0" // IR(31803, 18)
  "ImageQuerySize\0" // IR(31821, 15)
  "ImageQueryLod\0" // IR(31836, 14)
  "ImageQueryLevels\0" // IR(31850, 17)
  "ImageQuerySamples\0" // IR(31867, 18)
  "ConvertFToU\0" // IR(31885, 12)
  "ConvertFToS\0" // IR(31897, 12)
  "ConvertSToF\0" // IR(31909, 12)
  "ConvertUToF\0" // IR(31921, 12)
  "UConvert\0" // IR(31933, 9)
  "SConvert\0" // IR(31942, 9)
  "FConvert\0" // IR(31951, 9)
  "QuantizeToF16\0" // IR(31960, 14)
  "ConvertPtrToU\0" // IR(31974, 14)
  "SatConvertSToU\0" // IR(31988, 15)
  "SatConvertUToS\0" // IR(32003, 15)
  "ConvertUToPtr\0" // IR(32018, 14)
  "PtrCastToGeneric\0" // IR(32032, 17)
  "GenericCastToPtr\0" // IR(32049, 17)
  "GenericCastToPtrExplicit\0" // IR(32066, 25)
  "Bitcast\0" // IR(32091, 8)
  "SNegate\0" // IR(32099, 8)
  "FNegate\0" // IR(32107, 8)
  "IAdd\0" // IR(32115, 5)
  "FAdd\0" // IR(32120, 5)
  "ISub\0" // IR(32125, 5)
  "FSub\0" // IR(32130, 5)
  "IMul\0" // IR(32135, 5)
  "FMul\0" // IR(32140, 5)
  "UDiv\0" // IR(32145, 5)
  "SDiv\0" // IR(32150, 5)
  "FDiv\0" // IR(32155, 5)
  "UMod\0" // IR(32160, 5)
  "SRem\0" // IR(32165, 5)
  "SMod\0" // IR(32170, 5)
  "FRem\0" // IR(32175, 5)
  "FMod\0" // IR(32180, 5)
  "VectorTimesScalar\0" // IR(32185, 18)
  "MatrixTimesScalar\0" // IR(32203, 18)
  "VectorTimesMatrix\0" // IR(32221, 18)
  "MatrixTimesVector\0" // IR(32239, 18)
  "MatrixTimesMatrix\0" // IR(32257, 18)
  "OuterProduct\0" // IR(32275, 13)
  "Dot\0" // IR(32288, 4)
  "IAddCarry\0" // IR(32292, 10)
  "ISubBorrow\0" // IR(32302, 11)
  "UMulExtended\0" // IR(32313, 13)
  "SMulExtended\0" // IR(32326, 13)
  "Any\0" // IR(32339, 4)
  "All\0" // IR(32343, 4)
  "IsNan\0" // IR(32347, 6)
  "IsInf\0" // IR(32353, 6)
  "IsFinite\0" // IR(32359, 9)
  "IsNormal\0" // IR(32368, 9)
  "SignBitSet\0" // IR(32377, 11)
  "LessOrGreater\0" // IR(32388, 14)
  "Ordered\0" // IR(32402, 8)
  "Unordered\0" // IR(32410, 10)
  "LogicalEqual\0" // IR(32420, 13)
  "LogicalNotEqual\0" // IR(32433, 16)
  "LogicalOr\0" // IR(32449, 10)
  "LogicalAnd\0" // IR(32459, 11)
  "LogicalNot\0" // IR(32470, 11)
  "Select\0" // IR(32481, 7)
  "IEqual\0" // IR(32488, 7)
  "INotEqual\0" // IR(32495, 10)
  "UGreaterThan\0" // IR(32505, 13)
  "SGreaterThan\0" // IR(32518, 13)
  "UGreaterThanEqual\0" // IR(32531, 18)
  "SGreaterThanEqual\0" // IR(32549, 18)
  "ULessThan\0" // IR(32567, 10)
  "SLessThan\0" // IR(32577, 10)
  "ULessThanEqual\0" // IR(32587, 15)
  "SLessThanEqual\0" // IR(32602, 15)
  "FOrdEqual\0" // IR(32617, 10)
  "FUnordEqual\0" // IR(32627, 12)
  "FOrdNotEqual\0" // IR(32639, 13)
  "FUnordNotEqual\0" // IR(32652, 15)
  "FOrdLessThan\0" // IR(32667, 13)
  "FUnordLessThan\0" // IR(32680, 15)
  "FOrdGreaterThan\0" // IR(32695, 16)
  "FUnordGreaterThan\0" // IR(32711, 18)
  "FOrdLessThanEqual\0" // IR(32729, 18)
  "FUnordLessThanEqual\0" // IR(32747, 20)
  "FOrdGreaterThanEqual\0" // IR(32767, 21)
  "FUnordGreaterThanEqual\0" // IR(32788, 23)
  "ShiftRightLogical\0" // IR(32811, 18)
  "ShiftRightArithmetic\0" // IR(32829, 21)
  "ShiftLeftLogical\0" // IR(32850, 17)
  "BitwiseOr\0" // IR(32867, 10)
  "BitwiseXor\0" // IR(32877, 11)
  "BitwiseAnd\0" // IR(32888, 11)
  "Not\0" // IR(32899, 4)
  "BitFieldInsert\0" // IR(32903, 15)
  "BitFieldSExtract\0" // IR(32918, 17)
  "BitFieldUExtract\0" // IR(32935, 17)
  "BitReverse\0" // IR(32952, 11)
  "BitCount\0" // IR(32963, 9)
  "DPdx\0" // IR(32972, 5)
  "DPdy\0" // IR(32977, 5)
  "Fwidth\0" // IR(32982, 7)
  "DPdxFine\0" // IR(32989, 9)
  "DPdyFine\0" // IR(32998, 9)
  "FwidthFine\0" // IR(33007, 11)
  "DPdxCoarse\0" // IR(33018, 11)
  "DPdyCoarse\0" // IR(33029, 11)
  "FwidthCoarse\0" // IR(33040, 13)
  "EmitVertex\0" // IR(33053, 11)
  "EndPrimitive\0" // IR(33064, 13)
  "EmitStreamVertex\0" // IR(33077, 17)
  "EndStreamPrimitive\0" // IR(33094, 19)
  "SPV_OPERAND_TYPE_MEMORY_SEMANTICS_ID\0" // IR(33113, 37)
  "ControlBarrier\0" // IR(33150, 15)
  "MemoryBarrier\0" // IR(33165, 14)
  "AtomicLoad\0" // IR(33179, 11)
  "AtomicStore\0" // IR(33190, 12)
  "AtomicExchange\0" // IR(33202, 15)
  "AtomicCompareExchange\0" // IR(33217, 22)
  "AtomicCompareExchangeWeak\0" // IR(33239, 26)
  "AtomicIIncrement\0" // IR(33265, 17)
  "AtomicIDecrement\0" // IR(33282, 17)
  "AtomicIAdd\0" // IR(33299, 11)
  "AtomicISub\0" // IR(33310, 11)
  "AtomicSMin\0" // IR(33321, 11)
  "AtomicUMin\0" // IR(33332, 11)
  "AtomicSMax\0" // IR(33343, 11)
  "AtomicUMax\0" // IR(33354, 11)
  "AtomicAnd\0" // IR(33365, 10)
  "AtomicOr\0" // IR(33375, 9)
  "AtomicXor\0" // IR(33384, 10)
  "Phi\0" // IR(33394, 4)
  "SPV_OPERAND_TYPE_LOOP_CONTROL\0" // IR(33398, 30)
  "LoopMerge\0" // IR(33428, 10)
  "SPV_OPERAND_TYPE_SELECTION_CONTROL\0" // IR(33438, 35)
  "SelectionMerge\0" // IR(33473, 15)
  "Label\0" // IR(33488, 6)
  "Branch\0" // IR(33494, 7)
  "BranchConditional\0" // IR(33501, 18)
  "SPV_OPERAND_TYPE_VARIABLE_LITERAL_INTEGER_ID\0" // IR(33519, 45)
  "Switch\0" // IR(33564, 7)
  "Kill\0" // IR(33571, 5)
  "Return\0" // IR(33576, 7)
  "ReturnValue\0" // IR(33583, 12)
  "Unreachable\0" // IR(33595, 12)
  "LifetimeStart\0" // IR(33607, 14)
  "LifetimeStop\0" // IR(33621, 13)
  "GroupAsyncCopy\0" // IR(33634, 15)
  "GroupWaitEvents\0" // IR(33649, 16)
  "GroupAll\0" // IR(33665, 9)
  "GroupAny\0" // IR(33674, 9)
  "GroupBroadcast\0" // IR(33683, 15)
  "SPV_OPERAND_TYPE_GROUP_OPERATION\0" // IR(33698, 33)
  "GroupIAdd\0" // IR(33731, 10)
  "GroupFAdd\0" // IR(33741, 10)
  "GroupFMin\0" // IR(33751, 10)
  "GroupUMin\0" // IR(33761, 10)
  "GroupSMin\0" // IR(33771, 10)
  "GroupFMax\0" // IR(33781, 10)
  "GroupUMax\0" // IR(33791, 10)
  "GroupSMax\0" // IR(33801, 10)
  "ReadPipe\0" // IR(33811, 9)
  "WritePipe\0" // IR(33820, 10)
  "ReservedReadPipe\0" // IR(33830, 17)
  "ReservedWritePipe\0" // IR(33847, 18)
  "ReserveReadPipePackets\0" // IR(33865, 23)
  "ReserveWritePipePackets\0" // IR(33888, 24)
  "CommitReadPipe\0" // IR(33912, 15)
  "CommitWritePipe\0" // IR(33927, 16)
  "IsValidReserveId\0" // IR(33943, 17)
  "GetNumPipePackets\0" // IR(33960, 18)
  "GetMaxPipePackets\0" // IR(33978, 18)
  "GroupReserveReadPipePackets\0" // IR(33996, 28)
  "GroupReserveWritePipePackets\0" // IR(34024, 29)
  "GroupCommitReadPipe\0" // IR(34053, 20)
  "GroupCommitWritePipe\0" // IR(34073, 21)
  "EnqueueMarker\0" // IR(34094, 14)
  "EnqueueKernel\0" // IR(34108, 14)
  "GetKernelNDrangeSubGroupCount\0" // IR(34122, 30)
  "GetKernelNDrangeMaxSubGroupSize\0" // IR(34152, 32)
  "GetKernelWorkGroupSize\0" // IR(34184, 23)
  "GetKernelPreferredWorkGroupSizeMultiple\0" // IR(34207, 40)
  "RetainEvent\0" // IR(34247, 12)
  "ReleaseEvent\0" // IR(34259, 13)
  "CreateUserEvent\0" // IR(34272, 16)
  "IsValidEvent\0" // IR(34288, 13)
  "SetUserEventStatus\0" // IR(34301, 19)
  "CaptureEventProfilingInfo\0" // IR(34320, 26)
  "GetDefaultQueue\0" // IR(34346, 16)
  "BuildNDRange\0" // IR(34362, 13)
  "ImageSparseSampleImplicitLod\0" // IR(34375, 29)
  "ImageSparseSampleExplicitLod\0" // IR(34404, 29)
  "ImageSparseSampleDrefImplicitLod\0" // IR(34433, 33)
  "ImageSparseSampleDrefExplicitLod\0" // IR(34466, 33)
  "ImageSparseSampleProjImplicitLod\0" // IR(34499, 33)
  "ImageSparseSampleProjExplicitLod\0" // IR(34532, 33)
  "ImageSparseSampleProjDrefImplicitLod\0" // IR(34565, 37)
  "ImageSparseSampleProjDrefExplicitLod\0" // IR(34602, 37)
  "ImageSparseFetch\0" // IR(34639, 17)
  "ImageSparseGather\0" // IR(34656, 18)
  "ImageSparseDrefGather\0" // IR(34674, 22)
  "ImageSparseTexelsResident\0" // IR(34696, 26)
  "NoLine\0" // IR(34722, 7)
  "AtomicFlagTestAndSet\0" // IR(34729, 21)
  "AtomicFlagClear\0" // IR(34750, 16)
  "ImageSparseRead\0" // IR(34766, 16)
  "SizeOf\0" // IR(34782, 7)
  "TypePipeStorage\0" // IR(34789, 16)
  "ConstantPipeStorage\0" // IR(34805, 20)
  "CreatePipeFromPipeStorage\0" // IR(34825, 26)
  "GetKernelLocalSizeForSubgroupCount\0" // IR(34851, 35)
  "GetKernelMaxNumSubgroups\0" // IR(34886, 25)
  "TypeNamedBarrier\0" // IR(34911, 17)
  "NamedBarrierInitialize\0" // IR(34928, 23)
  "MemoryNamedBarrier\0" // IR(34951, 19)
  "ModuleProcessed\0" // IR(34970, 16)
  "ExecutionModeId\0" // IR(34986, 16)
  "DecorateId\0" // IR(35002, 11)
  "GroupNonUniformElect\0" // IR(35013, 21)
  "GroupNonUniformAll\0" // IR(35034, 19)
  "GroupNonUniformAny\0" // IR(35053, 19)
  "GroupNonUniformAllEqual\0" // IR(35072, 24)
  "GroupNonUniformBroadcast\0" // IR(35096, 25)
  "GroupNonUniformBroadcastFirst\0" // IR(35121, 30)
  "GroupNonUniformInverseBallot\0" // IR(35151, 29)
  "GroupNonUniformBallotBitExtract\0" // IR(35180, 32)
  "GroupNonUniformBallotBitCount\0" // IR(35212, 30)
  "GroupNonUniformBallotFindLSB\0" // IR(35242, 29)
  "GroupNonUniformBallotFindMSB\0" // IR(35271, 29)
  "GroupNonUniformShuffleXor\0" // IR(35300, 26)
  "GroupNonUniformShuffleUp\0" // IR(35326, 25)
  "GroupNonUniformShuffleDown\0" // IR(35351, 27)
  "GroupNonUniformIAdd\0" // IR(35378, 20)
  "GroupNonUniformFAdd\0" // IR(35398, 20)
  "GroupNonUniformIMul\0" // IR(35418, 20)
  "GroupNonUniformFMul\0" // IR(35438, 20)
  "GroupNonUniformSMin\0" // IR(35458, 20)
  "GroupNonUniformUMin\0" // IR(35478, 20)
  "GroupNonUniformFMin\0" // IR(35498, 20)
  "GroupNonUniformSMax\0" // IR(35518, 20)
  "GroupNonUniformUMax\0" // IR(35538, 20)
  "GroupNonUniformFMax\0" // IR(35558, 20)
  "GroupNonUniformBitwiseAnd\0" // IR(35578, 26)
  "GroupNonUniformBitwiseOr\0" // IR(35604, 25)
  "GroupNonUniformBitwiseXor\0" // IR(35629, 26)
  "GroupNonUniformLogicalAnd\0" // IR(35655, 26)
  "GroupNonUniformLogicalOr\0" // IR(35681, 25)
  "GroupNonUniformLogicalXor\0" // IR(35706, 26)
  "GroupNonUniformQuadBroadcast\0" // IR(35732, 29)
  "GroupNonUniformQuadSwap\0" // IR(35761, 24)
  "CopyLogical\0" // IR(35785, 12)
  "PtrEqual\0" // IR(35797, 9)
  "PtrNotEqual\0" // IR(35806, 12)
  "PtrDiff\0" // IR(35818, 8)
  "ColorAttachmentReadEXT\0" // IR(35826, 23)
  "DepthAttachmentReadEXT\0" // IR(35849, 23)
  "StencilAttachmentReadEXT\0" // IR(35872, 25)
  "TypeTensorARM\0" // IR(35897, 14)
  "SPV_OPERAND_TYPE_OPTIONAL_TENSOR_OPERANDS\0" // IR(35911, 42)
  "TensorReadARM\0" // IR(35953, 14)
  "TensorWriteARM\0" // IR(35967, 15)
  "TensorQuerySizeARM\0" // IR(35982, 19)
  "GraphConstantARM\0" // IR(36001, 17)
  "GraphEntryPointARM\0" // IR(36018, 19)
  "GraphInputARM\0" // IR(36037, 14)
  "GraphSetOutputARM\0" // IR(36051, 18)
  "GraphEndARM\0" // IR(36069, 12)
  "TypeGraphARM\0" // IR(36081, 13)
  "TerminateInvocation\0" // IR(36094, 20)
  "TypeUntypedPointerKHR\0" // IR(36114, 22)
  "UntypedVariableKHR\0" // IR(36136, 19)
  "UntypedAccessChainKHR\0" // IR(36155, 22)
  "UntypedInBoundsAccessChainKHR\0" // IR(36177, 30)
  "SubgroupFirstInvocationKHR\0" // IR(36207, 27)
  "UntypedPtrAccessChainKHR\0" // IR(36234, 25)
  "UntypedInBoundsPtrAccessChainKHR\0" // IR(36259, 33)
  "UntypedArrayLengthKHR\0" // IR(36292, 22)
  "UntypedPrefetchKHR\0" // IR(36314, 19)
  "FmaKHR\0" // IR(36333, 7)
  "SubgroupAllKHR\0" // IR(36340, 15)
  "SubgroupAnyKHR\0" // IR(36355, 15)
  "SubgroupAllEqualKHR\0" // IR(36370, 20)
  "SubgroupReadInvocationKHR\0" // IR(36390, 26)
  "ExtInstWithForwardRefsKHR\0" // IR(36416, 26)
  "UntypedGroupAsyncCopyKHR\0" // IR(36442, 25)
  "TraceRayKHR\0" // IR(36467, 12)
  "ExecuteCallableKHR\0" // IR(36479, 19)
  "ConvertUToAccelerationStructureKHR\0" // IR(36498, 35)
  "IgnoreIntersectionKHR\0" // IR(36533, 22)
  "TerminateRayKHR\0" // IR(36555, 16)
  "SPV_OPERAND_TYPE_OPTIONAL_PACKED_VECTOR_FORMAT\0" // IR(36571, 47)
  "SDot\0" // IR(36618, 5)
  "SDotKHR\0" // IR(36623, 8)
  "UDot\0" // IR(36631, 5)
  "UDotKHR\0" // IR(36636, 8)
  "SUDot\0" // IR(36644, 6)
  "SUDotKHR\0" // IR(36650, 9)
  "SDotAccSat\0" // IR(36659, 11)
  "SDotAccSatKHR\0" // IR(36670, 14)
  "UDotAccSat\0" // IR(36684, 11)
  "UDotAccSatKHR\0" // IR(36695, 14)
  "SUDotAccSat\0" // IR(36709, 12)
  "SUDotAccSatKHR\0" // IR(36721, 15)
  "TypeCooperativeMatrixKHR\0" // IR(36736, 25)
  "CooperativeMatrixLoadKHR\0" // IR(36761, 25)
  "CooperativeMatrixStoreKHR\0" // IR(36786, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_COOPERATIVE_MATRIX_OPERANDS\0" // IR(36812, 54)
  "CooperativeMatrixMulAddKHR\0" // IR(36866, 27)
  "CooperativeMatrixLengthKHR\0" // IR(36893, 27)
  "ConstantCompositeReplicateEXT\0" // IR(36920, 30)
  "SpecConstantCompositeReplicateEXT\0" // IR(36950, 34)
  "CompositeConstructReplicateEXT\0" // IR(36984, 31)
  "TypeRayQueryKHR\0" // IR(37015, 16)
  "RayQueryInitializeKHR\0" // IR(37031, 22)
  "RayQueryTerminateKHR\0" // IR(37053, 21)
  "RayQueryGenerateIntersectionKHR\0" // IR(37074, 32)
  "RayQueryConfirmIntersectionKHR\0" // IR(37106, 31)
  "RayQueryProceedKHR\0" // IR(37137, 19)
  "RayQueryGetIntersectionTypeKHR\0" // IR(37156, 31)
  "ImageSampleWeightedQCOM\0" // IR(37187, 24)
  "ImageBoxFilterQCOM\0" // IR(37211, 19)
  "ImageBlockMatchSSDQCOM\0" // IR(37230, 23)
  "ImageBlockMatchSADQCOM\0" // IR(37253, 23)
  "BitCastArrayQCOM\0" // IR(37276, 17)
  "ImageBlockMatchWindowSSDQCOM\0" // IR(37293, 29)
  "ImageBlockMatchWindowSADQCOM\0" // IR(37322, 29)
  "ImageBlockMatchGatherSSDQCOM\0" // IR(37351, 29)
  "ImageBlockMatchGatherSADQCOM\0" // IR(37380, 29)
  "CompositeConstructCoopMatQCOM\0" // IR(37409, 30)
  "CompositeExtractCoopMatQCOM\0" // IR(37439, 28)
  "ExtractSubArrayQCOM\0" // IR(37467, 20)
  "ImageGatherQCOM\0" // IR(37487, 16)
  "GroupIAddNonUniformAMD\0" // IR(37503, 23)
  "GroupFAddNonUniformAMD\0" // IR(37526, 23)
  "GroupFMinNonUniformAMD\0" // IR(37549, 23)
  "GroupUMinNonUniformAMD\0" // IR(37572, 23)
  "GroupSMinNonUniformAMD\0" // IR(37595, 23)
  "GroupFMaxNonUniformAMD\0" // IR(37618, 23)
  "GroupUMaxNonUniformAMD\0" // IR(37641, 23)
  "GroupSMaxNonUniformAMD\0" // IR(37664, 23)
  "FragmentMaskFetchAMD\0" // IR(37687, 21)
  "FragmentFetchAMD\0" // IR(37708, 17)
  "ReadClockKHR\0" // IR(37725, 13)
  "AllocateNodePayloadsAMDX\0" // IR(37738, 25)
  "EnqueueNodePayloadsAMDX\0" // IR(37763, 24)
  "TypeNodePayloadArrayAMDX\0" // IR(37787, 25)
  "FinishWritingNodePayloadAMDX\0" // IR(37812, 29)
  "NodePayloadArrayLengthAMDX\0" // IR(37841, 27)
  "IsNodePayloadValidAMDX\0" // IR(37868, 23)
  "ConstantStringAMDX\0" // IR(37891, 19)
  "SpecConstantStringAMDX\0" // IR(37910, 23)
  "GroupNonUniformQuadAllKHR\0" // IR(37933, 26)
  "GroupNonUniformQuadAnyKHR\0" // IR(37959, 26)
  "TypeBufferEXT\0" // IR(37985, 14)
  "BufferPointerEXT\0" // IR(37999, 17)
  "UntypedImageTexelPointerEXT\0" // IR(38016, 28)
  "MemberDecorateIdEXT\0" // IR(38044, 20)
  "ConstantSizeOfEXT\0" // IR(38064, 18)
  "SpecConstantDataKHR\0" // IR(38082, 20)
  "PoisonKHR\0" // IR(38102, 10)
  "FreezeKHR\0" // IR(38112, 10)
  "HitObjectRecordHitMotionNV\0" // IR(38122, 27)
  "HitObjectRecordHitWithIndexMotionNV\0" // IR(38149, 36)
  "HitObjectRecordMissMotionNV\0" // IR(38185, 28)
  "HitObjectGetWorldToObjectNV\0" // IR(38213, 28)
  "HitObjectGetObjectToWorldNV\0" // IR(38241, 28)
  "HitObjectGetObjectRayDirectionNV\0" // IR(38269, 33)
  "HitObjectGetObjectRayOriginNV\0" // IR(38302, 30)
  "HitObjectTraceRayMotionNV\0" // IR(38332, 26)
  "HitObjectGetShaderRecordBufferHandleNV\0" // IR(38358, 39)
  "HitObjectGetShaderBindingTableRecordIndexNV\0" // IR(38397, 44)
  "HitObjectRecordEmptyNV\0" // IR(38441, 23)
  "HitObjectTraceRayNV\0" // IR(38464, 20)
  "HitObjectRecordHitNV\0" // IR(38484, 21)
  "HitObjectRecordHitWithIndexNV\0" // IR(38505, 30)
  "HitObjectRecordMissNV\0" // IR(38535, 22)
  "HitObjectExecuteShaderNV\0" // IR(38557, 25)
  "HitObjectGetCurrentTimeNV\0" // IR(38582, 26)
  "HitObjectGetAttributesNV\0" // IR(38608, 25)
  "HitObjectGetHitKindNV\0" // IR(38633, 22)
  "HitObjectGetPrimitiveIndexNV\0" // IR(38655, 29)
  "HitObjectGetGeometryIndexNV\0" // IR(38684, 28)
  "HitObjectGetInstanceIdNV\0" // IR(38712, 25)
  "HitObjectGetInstanceCustomIndexNV\0" // IR(38737, 34)
  "HitObjectGetWorldRayDirectionNV\0" // IR(38771, 32)
  "HitObjectGetWorldRayOriginNV\0" // IR(38803, 29)
  "HitObjectGetRayTMaxNV\0" // IR(38832, 22)
  "HitObjectGetRayTMinNV\0" // IR(38854, 22)
  "HitObjectIsEmptyNV\0" // IR(38876, 19)
  "HitObjectIsHitNV\0" // IR(38895, 17)
  "HitObjectIsMissNV\0" // IR(38912, 18)
  "ReorderThreadWithHitObjectNV\0" // IR(38930, 29)
  "ReorderThreadWithHintNV\0" // IR(38959, 24)
  "TypeHitObjectNV\0" // IR(38983, 16)
  "ImageSampleFootprintNV\0" // IR(38999, 23)
  "TypeVectorIdEXT\0" // IR(39022, 16)
  "TypeCooperativeVectorNV\0" // IR(39038, 24)
  "CooperativeVectorMatrixMulNV\0" // IR(39062, 29)
  "CooperativeVectorOuterProductAccumulateNV\0" // IR(39091, 42)
  "CooperativeVectorReduceSumAccumulateNV\0" // IR(39133, 39)
  "CooperativeVectorMatrixMulAddNV\0" // IR(39172, 32)
  "CooperativeMatrixConvertUseEXT\0" // IR(39204, 31)
  "CooperativeMatrixConvertNV\0" // IR(39235, 27)
  "EmitMeshTasksEXT\0" // IR(39262, 17)
  "SetMeshOutputsEXT\0" // IR(39279, 18)
  "GroupNonUniformPartitionEXT\0" // IR(39297, 28)
  "GroupNonUniformPartitionNV\0" // IR(39325, 27)
  "WritePackedPrimitiveIndices4x8NV\0" // IR(39352, 33)
  "FetchMicroTriangleVertexPositionNV\0" // IR(39385, 35)
  "FetchMicroTriangleVertexBarycentricNV\0" // IR(39420, 38)
  "CooperativeVectorLoadNV\0" // IR(39458, 24)
  "CooperativeVectorStoreNV\0" // IR(39482, 25)
  "HitObjectRecordFromQueryEXT\0" // IR(39507, 28)
  "HitObjectRecordMissEXT\0" // IR(39535, 23)
  "HitObjectRecordMissMotionEXT\0" // IR(39558, 29)
  "HitObjectGetIntersectionTriangleVertexPositionsEXT\0" // IR(39587, 51)
  "HitObjectGetRayFlagsEXT\0" // IR(39638, 24)
  "HitObjectSetShaderBindingTableRecordIndexEXT\0" // IR(39662, 45)
  "HitObjectReorderExecuteShaderEXT\0" // IR(39707, 33)
  "HitObjectTraceReorderExecuteEXT\0" // IR(39740, 32)
  "HitObjectTraceMotionReorderExecuteEXT\0" // IR(39772, 38)
  "TypeHitObjectEXT\0" // IR(39810, 17)
  "ReorderThreadWithHintEXT\0" // IR(39827, 25)
  "ReorderThreadWithHitObjectEXT\0" // IR(39852, 30)
  "HitObjectTraceRayEXT\0" // IR(39882, 21)
  "HitObjectTraceRayMotionEXT\0" // IR(39903, 27)
  "HitObjectRecordEmptyEXT\0" // IR(39930, 24)
  "HitObjectExecuteShaderEXT\0" // IR(39954, 26)
  "HitObjectGetCurrentTimeEXT\0" // IR(39980, 27)
  "HitObjectGetAttributesEXT\0" // IR(40007, 26)
  "HitObjectGetHitKindEXT\0" // IR(40033, 23)
  "HitObjectGetPrimitiveIndexEXT\0" // IR(40056, 30)
  "HitObjectGetGeometryIndexEXT\0" // IR(40086, 29)
  "HitObjectGetInstanceIdEXT\0" // IR(40115, 26)
  "HitObjectGetInstanceCustomIndexEXT\0" // IR(40141, 35)
  "HitObjectGetObjectRayOriginEXT\0" // IR(40176, 31)
  "HitObjectGetObjectRayDirectionEXT\0" // IR(40207, 34)
  "HitObjectGetWorldRayDirectionEXT\0" // IR(40241, 33)
  "HitObjectGetWorldRayOriginEXT\0" // IR(40274, 30)
  "HitObjectGetObjectToWorldEXT\0" // IR(40304, 29)
  "HitObjectGetWorldToObjectEXT\0" // IR(40333, 29)
  "HitObjectGetRayTMaxEXT\0" // IR(40362, 23)
  "ReportIntersectionKHR\0" // IR(40385, 22)
  "ReportIntersectionNV\0" // IR(40407, 21)
  "IgnoreIntersectionNV\0" // IR(40428, 21)
  "TerminateRayNV\0" // IR(40449, 15)
  "TraceNV\0" // IR(40464, 8)
  "TraceMotionNV\0" // IR(40472, 14)
  "TraceRayMotionNV\0" // IR(40486, 17)
  "RayQueryGetIntersectionTriangleVertexPositionsKHR\0" // IR(40503, 50)
  "TypeAccelerationStructureKHR\0" // IR(40553, 29)
  "TypeAccelerationStructureNV\0" // IR(40582, 28)
  "ExecuteCallableNV\0" // IR(40610, 18)
  "RayQueryGetIntersectionClusterIdNV\0" // IR(40628, 35)
  "RayQueryGetClusterIdNV\0" // IR(40663, 23)
  "HitObjectGetClusterIdNV\0" // IR(40686, 24)
  "HitObjectGetRayTMinEXT\0" // IR(40710, 23)
  "HitObjectGetShaderBindingTableRecordIndexEXT\0" // IR(40733, 45)
  "HitObjectGetShaderRecordBufferHandleEXT\0" // IR(40778, 40)
  "HitObjectIsEmptyEXT\0" // IR(40818, 20)
  "HitObjectIsHitEXT\0" // IR(40838, 18)
  "HitObjectIsMissEXT\0" // IR(40856, 19)
  "TypeCooperativeMatrixNV\0" // IR(40875, 24)
  "CooperativeMatrixLoadNV\0" // IR(40899, 24)
  "CooperativeMatrixStoreNV\0" // IR(40923, 25)
  "CooperativeMatrixMulAddNV\0" // IR(40948, 26)
  "CooperativeMatrixLengthNV\0" // IR(40974, 26)
  "BeginInvocationInterlockEXT\0" // IR(41000, 28)
  "EndInvocationInterlockEXT\0" // IR(41028, 26)
  "SPV_OPERAND_TYPE_COOPERATIVE_MATRIX_REDUCE\0" // IR(41054, 43)
  "CooperativeMatrixReduceEXT\0" // IR(41097, 27)
  "CooperativeMatrixReduceNV\0" // IR(41124, 26)
  "SPV_OPERAND_TYPE_MEMORY_ACCESS\0" // IR(41150, 31)
  "SPV_OPERAND_TYPE_TENSOR_ADDRESSING_OPERANDS\0" // IR(41181, 44)
  "CooperativeMatrixLoadTensorNV\0" // IR(41225, 30)
  "CooperativeMatrixStoreTensorNV\0" // IR(41255, 31)
  "CooperativeMatrixPerElementOpEXT\0" // IR(41286, 33)
  "CooperativeMatrixPerElementOpNV\0" // IR(41319, 32)
  "TypeTensorLayoutNV\0" // IR(41351, 19)
  "TypeTensorViewNV\0" // IR(41370, 17)
  "CreateTensorLayoutNV\0" // IR(41387, 21)
  "TensorLayoutSetDimensionNV\0" // IR(41408, 27)
  "TensorLayoutSetStrideNV\0" // IR(41435, 24)
  "TensorLayoutSliceNV\0" // IR(41459, 20)
  "TensorLayoutSetClampValueNV\0" // IR(41479, 28)
  "CreateTensorViewNV\0" // IR(41507, 19)
  "TensorViewSetDimensionNV\0" // IR(41526, 25)
  "TensorViewSetStrideNV\0" // IR(41551, 22)
  "IsHelperInvocationEXT\0" // IR(41573, 22)
  "TensorViewSetClipNV\0" // IR(41595, 20)
  "TensorLayoutSetBlockSizeNV\0" // IR(41615, 27)
  "CooperativeMatrixTransposeNV\0" // IR(41642, 29)
  "ConvertUToImageNV\0" // IR(41671, 18)
  "ConvertUToSamplerNV\0" // IR(41689, 20)
  "ConvertImageToUNV\0" // IR(41709, 18)
  "ConvertSamplerToUNV\0" // IR(41727, 20)
  "ConvertUToSampledImageNV\0" // IR(41747, 25)
  "ConvertSampledImageToUNV\0" // IR(41772, 25)
  "SamplerImageAddressingModeNV\0" // IR(41797, 29)
  "SPV_OPERAND_TYPE_OPTIONAL_RAW_ACCESS_CHAIN_OPERANDS\0" // IR(41826, 52)
  "RawAccessChainNV\0" // IR(41878, 17)
  "RayQueryGetIntersectionSpherePositionNV\0" // IR(41895, 40)
  "RayQueryGetIntersectionSphereRadiusNV\0" // IR(41935, 38)
  "RayQueryGetIntersectionLSSPositionsNV\0" // IR(41973, 38)
  "RayQueryGetIntersectionLSSRadiiNV\0" // IR(42011, 34)
  "RayQueryGetIntersectionLSSHitValueNV\0" // IR(42045, 37)
  "HitObjectGetSpherePositionNV\0" // IR(42082, 29)
  "HitObjectGetSphereRadiusNV\0" // IR(42111, 27)
  "HitObjectGetLSSPositionsNV\0" // IR(42138, 27)
  "HitObjectGetLSSRadiiNV\0" // IR(42165, 23)
  "HitObjectIsSphereHitNV\0" // IR(42188, 23)
  "HitObjectIsLSSHitNV\0" // IR(42211, 20)
  "RayQueryIsSphereHitNV\0" // IR(42231, 22)
  "RayQueryIsLSSHitNV\0" // IR(42253, 19)
  "SubgroupShuffleDownINTEL\0" // IR(42272, 25)
  "SubgroupShuffleUpINTEL\0" // IR(42297, 23)
  "SubgroupShuffleXorINTEL\0" // IR(42320, 24)
  "SubgroupBlockReadINTEL\0" // IR(42344, 23)
  "SubgroupBlockWriteINTEL\0" // IR(42367, 24)
  "SubgroupImageBlockReadINTEL\0" // IR(42391, 28)
  "SubgroupImageBlockWriteINTEL\0" // IR(42419, 29)
  "SubgroupImageMediaBlockReadINTEL\0" // IR(42448, 33)
  "SubgroupImageMediaBlockWriteINTEL\0" // IR(42481, 34)
  "UCountLeadingZerosINTEL\0" // IR(42515, 24)
  "UCountTrailingZerosINTEL\0" // IR(42539, 25)
  "AbsISubINTEL\0" // IR(42564, 13)
  "AbsUSubINTEL\0" // IR(42577, 13)
  "IAddSatINTEL\0" // IR(42590, 13)
  "UAddSatINTEL\0" // IR(42603, 13)
  "IAverageINTEL\0" // IR(42616, 14)
  "UAverageINTEL\0" // IR(42630, 14)
  "IAverageRoundedINTEL\0" // IR(42644, 21)
  "UAverageRoundedINTEL\0" // IR(42665, 21)
  "ISubSatINTEL\0" // IR(42686, 13)
  "USubSatINTEL\0" // IR(42699, 13)
  "IMul32x16INTEL\0" // IR(42712, 15)
  "UMul32x16INTEL\0" // IR(42727, 15)
  "ConstantFunctionPointerINTEL\0" // IR(42742, 29)
  "FunctionPointerCallINTEL\0" // IR(42771, 25)
  "AsmTargetINTEL\0" // IR(42796, 15)
  "AsmCallINTEL\0" // IR(42811, 13)
  "AtomicFMinEXT\0" // IR(42824, 14)
  "AtomicFMaxEXT\0" // IR(42838, 14)
  "AssumeTrueKHR\0" // IR(42852, 14)
  "ExpectKHR\0" // IR(42866, 10)
  "DecorateString\0" // IR(42876, 15)
  "DecorateStringGOOGLE\0" // IR(42891, 21)
  "MemberDecorateString\0" // IR(42912, 21)
  "MemberDecorateStringGOOGLE\0" // IR(42933, 27)
  "VmeImageINTEL\0" // IR(42960, 14)
  "TypeVmeImageINTEL\0" // IR(42974, 18)
  "TypeAvcImePayloadINTEL\0" // IR(42992, 23)
  "TypeAvcRefPayloadINTEL\0" // IR(43015, 23)
  "TypeAvcSicPayloadINTEL\0" // IR(43038, 23)
  "TypeAvcMcePayloadINTEL\0" // IR(43061, 23)
  "TypeAvcMceResultINTEL\0" // IR(43084, 22)
  "TypeAvcImeResultINTEL\0" // IR(43106, 22)
  "TypeAvcImeResultSingleReferenceStreamoutINTEL\0" // IR(43128, 46)
  "TypeAvcImeResultDualReferenceStreamoutINTEL\0" // IR(43174, 44)
  "TypeAvcImeSingleReferenceStreaminINTEL\0" // IR(43218, 39)
  "TypeAvcImeDualReferenceStreaminINTEL\0" // IR(43257, 37)
  "TypeAvcRefResultINTEL\0" // IR(43294, 22)
  "TypeAvcSicResultINTEL\0" // IR(43316, 22)
  "SubgroupAvcMceGetDefaultInterBaseMultiReferencePenaltyINTEL\0" // IR(43338, 60)
  "SubgroupAvcMceSetInterBaseMultiReferencePenaltyINTEL\0" // IR(43398, 53)
  "SubgroupAvcMceGetDefaultInterShapePenaltyINTEL\0" // IR(43451, 47)
  "SubgroupAvcMceSetInterShapePenaltyINTEL\0" // IR(43498, 40)
  "SubgroupAvcMceGetDefaultInterDirectionPenaltyINTEL\0" // IR(43538, 51)
  "SubgroupAvcMceSetInterDirectionPenaltyINTEL\0" // IR(43589, 44)
  "SubgroupAvcMceGetDefaultIntraLumaShapePenaltyINTEL\0" // IR(43633, 51)
  "SubgroupAvcMceGetDefaultInterMotionVectorCostTableINTEL\0" // IR(43684, 56)
  "SubgroupAvcMceGetDefaultHighPenaltyCostTableINTEL\0" // IR(43740, 50)
  "SubgroupAvcMceGetDefaultMediumPenaltyCostTableINTEL\0" // IR(43790, 52)
  "SubgroupAvcMceGetDefaultLowPenaltyCostTableINTEL\0" // IR(43842, 49)
  "SubgroupAvcMceSetMotionVectorCostFunctionINTEL\0" // IR(43891, 47)
  "SubgroupAvcMceGetDefaultIntraLumaModePenaltyINTEL\0" // IR(43938, 50)
  "SubgroupAvcMceGetDefaultNonDcLumaIntraPenaltyINTEL\0" // IR(43988, 51)
  "SubgroupAvcMceGetDefaultIntraChromaModeBasePenaltyINTEL\0" // IR(44039, 56)
  "SubgroupAvcMceSetAcOnlyHaarINTEL\0" // IR(44095, 33)
  "SubgroupAvcMceSetSourceInterlacedFieldPolarityINTEL\0" // IR(44128, 52)
  "SubgroupAvcMceSetSingleReferenceInterlacedFieldPolarityINTEL\0" // IR(44180, 61)
  "SubgroupAvcMceSetDualReferenceInterlacedFieldPolaritiesINTEL\0" // IR(44241, 61)
  "SubgroupAvcMceConvertToImePayloadINTEL\0" // IR(44302, 39)
  "SubgroupAvcMceConvertToImeResultINTEL\0" // IR(44341, 38)
  "SubgroupAvcMceConvertToRefPayloadINTEL\0" // IR(44379, 39)
  "SubgroupAvcMceConvertToRefResultINTEL\0" // IR(44418, 38)
  "SubgroupAvcMceConvertToSicPayloadINTEL\0" // IR(44456, 39)
  "SubgroupAvcMceConvertToSicResultINTEL\0" // IR(44495, 38)
  "SubgroupAvcMceGetMotionVectorsINTEL\0" // IR(44533, 36)
  "SubgroupAvcMceGetInterDistortionsINTEL\0" // IR(44569, 39)
  "SubgroupAvcMceGetBestInterDistortionsINTEL\0" // IR(44608, 43)
  "SubgroupAvcMceGetInterMajorShapeINTEL\0" // IR(44651, 38)
  "SubgroupAvcMceGetInterMinorShapeINTEL\0" // IR(44689, 38)
  "SubgroupAvcMceGetInterDirectionsINTEL\0" // IR(44727, 38)
  "SubgroupAvcMceGetInterMotionVectorCountINTEL\0" // IR(44765, 45)
  "SubgroupAvcMceGetInterReferenceIdsINTEL\0" // IR(44810, 40)
  "SubgroupAvcMceGetInterReferenceInterlacedFieldPolaritiesINTEL\0" // IR(44850, 62)
  "SubgroupAvcImeInitializeINTEL\0" // IR(44912, 30)
  "SubgroupAvcImeSetSingleReferenceINTEL\0" // IR(44942, 38)
  "SubgroupAvcImeSetDualReferenceINTEL\0" // IR(44980, 36)
  "SubgroupAvcImeRefWindowSizeINTEL\0" // IR(45016, 33)
  "SubgroupAvcImeAdjustRefOffsetINTEL\0" // IR(45049, 35)
  "SubgroupAvcImeConvertToMcePayloadINTEL\0" // IR(45084, 39)
  "SubgroupAvcImeSetMaxMotionVectorCountINTEL\0" // IR(45123, 43)
  "SubgroupAvcImeSetUnidirectionalMixDisableINTEL\0" // IR(45166, 47)
  "SubgroupAvcImeSetEarlySearchTerminationThresholdINTEL\0" // IR(45213, 54)
  "SubgroupAvcImeSetWeightedSadINTEL\0" // IR(45267, 34)
  "SubgroupAvcImeEvaluateWithSingleReferenceINTEL\0" // IR(45301, 47)
  "SubgroupAvcImeEvaluateWithDualReferenceINTEL\0" // IR(45348, 45)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminINTEL\0" // IR(45393, 55)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminINTEL\0" // IR(45448, 53)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreamoutINTEL\0" // IR(45501, 56)
  "SubgroupAvcImeEvaluateWithDualReferenceStreamoutINTEL\0" // IR(45557, 54)
  "SubgroupAvcImeEvaluateWithSingleReferenceStreaminoutINTEL\0" // IR(45611, 58)
  "SubgroupAvcImeEvaluateWithDualReferenceStreaminoutINTEL\0" // IR(45669, 56)
  "SubgroupAvcImeConvertToMceResultINTEL\0" // IR(45725, 38)
  "SubgroupAvcImeGetSingleReferenceStreaminINTEL\0" // IR(45763, 46)
  "SubgroupAvcImeGetDualReferenceStreaminINTEL\0" // IR(45809, 44)
  "SubgroupAvcImeStripSingleReferenceStreamoutINTEL\0" // IR(45853, 49)
  "SubgroupAvcImeStripDualReferenceStreamoutINTEL\0" // IR(45902, 47)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeMotionVectorsINTEL\0" // IR(45949, 70)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeDistortionsINTEL\0" // IR(46019, 68)
  "SubgroupAvcImeGetStreamoutSingleReferenceMajorShapeReferenceIdsINTEL\0" // IR(46087, 69)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeMotionVectorsINTEL\0" // IR(46156, 68)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeDistortionsINTEL\0" // IR(46224, 66)
  "SubgroupAvcImeGetStreamoutDualReferenceMajorShapeReferenceIdsINTEL\0" // IR(46290, 67)
  "SubgroupAvcImeGetBorderReachedINTEL\0" // IR(46357, 36)
  "SubgroupAvcImeGetTruncatedSearchIndicationINTEL\0" // IR(46393, 48)
  "SubgroupAvcImeGetUnidirectionalEarlySearchTerminationINTEL\0" // IR(46441, 59)
  "SubgroupAvcImeGetWeightingPatternMinimumMotionVectorINTEL\0" // IR(46500, 58)
  "SubgroupAvcImeGetWeightingPatternMinimumDistortionINTEL\0" // IR(46558, 56)
  "SubgroupAvcFmeInitializeINTEL\0" // IR(46614, 30)
  "SubgroupAvcBmeInitializeINTEL\0" // IR(46644, 30)
  "SubgroupAvcRefConvertToMcePayloadINTEL\0" // IR(46674, 39)
  "SubgroupAvcRefSetBidirectionalMixDisableINTEL\0" // IR(46713, 46)
  "SubgroupAvcRefSetBilinearFilterEnableINTEL\0" // IR(46759, 43)
  "SubgroupAvcRefEvaluateWithSingleReferenceINTEL\0" // IR(46802, 47)
  "SubgroupAvcRefEvaluateWithDualReferenceINTEL\0" // IR(46849, 45)
  "SubgroupAvcRefEvaluateWithMultiReferenceINTEL\0" // IR(46894, 46)
  "SubgroupAvcRefEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(46940, 56)
  "SubgroupAvcRefConvertToMceResultINTEL\0" // IR(46996, 38)
  "SubgroupAvcSicInitializeINTEL\0" // IR(47034, 30)
  "SubgroupAvcSicConfigureSkcINTEL\0" // IR(47064, 32)
  "SubgroupAvcSicConfigureIpeLumaINTEL\0" // IR(47096, 36)
  "SubgroupAvcSicConfigureIpeLumaChromaINTEL\0" // IR(47132, 42)
  "SubgroupAvcSicGetMotionVectorMaskINTEL\0" // IR(47174, 39)
  "SubgroupAvcSicConvertToMcePayloadINTEL\0" // IR(47213, 39)
  "SubgroupAvcSicSetIntraLumaShapePenaltyINTEL\0" // IR(47252, 44)
  "SubgroupAvcSicSetIntraLumaModeCostFunctionINTEL\0" // IR(47296, 48)
  "SubgroupAvcSicSetIntraChromaModeCostFunctionINTEL\0" // IR(47344, 50)
  "SubgroupAvcSicSetBilinearFilterEnableINTEL\0" // IR(47394, 43)
  "SubgroupAvcSicSetSkcForwardTransformEnableINTEL\0" // IR(47437, 48)
  "SubgroupAvcSicSetBlockBasedRawSkipSadINTEL\0" // IR(47485, 43)
  "SubgroupAvcSicEvaluateIpeINTEL\0" // IR(47528, 31)
  "SubgroupAvcSicEvaluateWithSingleReferenceINTEL\0" // IR(47559, 47)
  "SubgroupAvcSicEvaluateWithDualReferenceINTEL\0" // IR(47606, 45)
  "SubgroupAvcSicEvaluateWithMultiReferenceINTEL\0" // IR(47651, 46)
  "SubgroupAvcSicEvaluateWithMultiReferenceInterlacedINTEL\0" // IR(47697, 56)
  "SubgroupAvcSicConvertToMceResultINTEL\0" // IR(47753, 38)
  "SubgroupAvcSicGetIpeLumaShapeINTEL\0" // IR(47791, 35)
  "SubgroupAvcSicGetBestIpeLumaDistortionINTEL\0" // IR(47826, 44)
  "SubgroupAvcSicGetBestIpeChromaDistortionINTEL\0" // IR(47870, 46)
  "SubgroupAvcSicGetPackedIpeLumaModesINTEL\0" // IR(47916, 41)
  "SubgroupAvcSicGetIpeChromaModeINTEL\0" // IR(47957, 36)
  "SubgroupAvcSicGetPackedSkcLumaCountThresholdINTEL\0" // IR(47993, 50)
  "SubgroupAvcSicGetPackedSkcLumaSumThresholdINTEL\0" // IR(48043, 48)
  "SubgroupAvcSicGetInterRawSadsINTEL\0" // IR(48091, 35)
  "SaveMemoryINTEL\0" // IR(48126, 16)
  "RestoreMemoryINTEL\0" // IR(48142, 19)
  "ArbitraryFloatSinCosPiALTERA\0" // IR(48161, 29)
  "ArbitraryFloatSinCosPiINTEL\0" // IR(48190, 28)
  "ArbitraryFloatCastALTERA\0" // IR(48218, 25)
  "ArbitraryFloatCastINTEL\0" // IR(48243, 24)
  "ArbitraryFloatCastFromIntALTERA\0" // IR(48267, 32)
  "ArbitraryFloatCastFromIntINTEL\0" // IR(48299, 31)
  "ArbitraryFloatCastToIntALTERA\0" // IR(48330, 30)
  "ArbitraryFloatCastToIntINTEL\0" // IR(48360, 29)
  "ArbitraryFloatAddALTERA\0" // IR(48389, 24)
  "ArbitraryFloatAddINTEL\0" // IR(48413, 23)
  "ArbitraryFloatSubALTERA\0" // IR(48436, 24)
  "ArbitraryFloatSubINTEL\0" // IR(48460, 23)
  "ArbitraryFloatMulALTERA\0" // IR(48483, 24)
  "ArbitraryFloatMulINTEL\0" // IR(48507, 23)
  "ArbitraryFloatDivALTERA\0" // IR(48530, 24)
  "ArbitraryFloatDivINTEL\0" // IR(48554, 23)
  "ArbitraryFloatGTALTERA\0" // IR(48577, 23)
  "ArbitraryFloatGTINTEL\0" // IR(48600, 22)
  "ArbitraryFloatGEALTERA\0" // IR(48622, 23)
  "ArbitraryFloatGEINTEL\0" // IR(48645, 22)
  "ArbitraryFloatLTALTERA\0" // IR(48667, 23)
  "ArbitraryFloatLTINTEL\0" // IR(48690, 22)
  "ArbitraryFloatLEALTERA\0" // IR(48712, 23)
  "ArbitraryFloatLEINTEL\0" // IR(48735, 22)
  "ArbitraryFloatEQALTERA\0" // IR(48757, 23)
  "ArbitraryFloatEQINTEL\0" // IR(48780, 22)
  "ArbitraryFloatRecipALTERA\0" // IR(48802, 26)
  "ArbitraryFloatRecipINTEL\0" // IR(48828, 25)
  "ArbitraryFloatRSqrtALTERA\0" // IR(48853, 26)
  "ArbitraryFloatRSqrtINTEL\0" // IR(48879, 25)
  "ArbitraryFloatCbrtALTERA\0" // IR(48904, 25)
  "ArbitraryFloatCbrtINTEL\0" // IR(48929, 24)
  "ArbitraryFloatHypotALTERA\0" // IR(48953, 26)
  "ArbitraryFloatHypotINTEL\0" // IR(48979, 25)
  "ArbitraryFloatSqrtALTERA\0" // IR(49004, 25)
  "ArbitraryFloatSqrtINTEL\0" // IR(49029, 24)
  "ArbitraryFloatLogINTEL\0" // IR(49053, 23)
  "ArbitraryFloatLog2INTEL\0" // IR(49076, 24)
  "ArbitraryFloatLog10INTEL\0" // IR(49100, 25)
  "ArbitraryFloatLog1pINTEL\0" // IR(49125, 25)
  "ArbitraryFloatExpINTEL\0" // IR(49150, 23)
  "ArbitraryFloatExp2INTEL\0" // IR(49173, 24)
  "ArbitraryFloatExp10INTEL\0" // IR(49197, 25)
  "ArbitraryFloatExpm1INTEL\0" // IR(49222, 25)
  "ArbitraryFloatSinINTEL\0" // IR(49247, 23)
  "ArbitraryFloatCosINTEL\0" // IR(49270, 23)
  "ArbitraryFloatSinCosINTEL\0" // IR(49293, 26)
  "ArbitraryFloatSinPiINTEL\0" // IR(49319, 25)
  "ArbitraryFloatCosPiINTEL\0" // IR(49344, 25)
  "ArbitraryFloatASinINTEL\0" // IR(49369, 24)
  "ArbitraryFloatASinPiINTEL\0" // IR(49393, 26)
  "ArbitraryFloatACosINTEL\0" // IR(49419, 24)
  "ArbitraryFloatACosPiINTEL\0" // IR(49443, 26)
  "ArbitraryFloatATanINTEL\0" // IR(49469, 24)
  "ArbitraryFloatATanPiINTEL\0" // IR(49493, 26)
  "ArbitraryFloatATan2INTEL\0" // IR(49519, 25)
  "ArbitraryFloatPowINTEL\0" // IR(49544, 23)
  "ArbitraryFloatPowRINTEL\0" // IR(49567, 24)
  "ArbitraryFloatPowNINTEL\0" // IR(49591, 24)
  "LoopControlINTEL\0" // IR(49615, 17)
  "AliasDomainDeclINTEL\0" // IR(49632, 21)
  "AliasScopeDeclINTEL\0" // IR(49653, 20)
  "AliasScopeListDeclINTEL\0" // IR(49673, 24)
  "FixedSqrtALTERA\0" // IR(49697, 16)
  "FixedSqrtINTEL\0" // IR(49713, 15)
  "FixedRecipALTERA\0" // IR(49728, 17)
  "FixedRecipINTEL\0" // IR(49745, 16)
  "FixedRsqrtALTERA\0" // IR(49761, 17)
  "FixedRsqrtINTEL\0" // IR(49778, 16)
  "FixedSinALTERA\0" // IR(49794, 15)
  "FixedSinINTEL\0" // IR(49809, 14)
  "FixedCosALTERA\0" // IR(49823, 15)
  "FixedCosINTEL\0" // IR(49838, 14)
  "FixedSinCosALTERA\0" // IR(49852, 18)
  "FixedSinCosINTEL\0" // IR(49870, 17)
  "FixedSinPiALTERA\0" // IR(49887, 17)
  "FixedSinPiINTEL\0" // IR(49904, 16)
  "FixedCosPiALTERA\0" // IR(49920, 17)
  "FixedCosPiINTEL\0" // IR(49937, 16)
  "FixedSinCosPiALTERA\0" // IR(49953, 20)
  "FixedSinCosPiINTEL\0" // IR(49973, 19)
  "FixedLogALTERA\0" // IR(49992, 15)
  "FixedLogINTEL\0" // IR(50007, 14)
  "FixedExpALTERA\0" // IR(50021, 15)
  "FixedExpINTEL\0" // IR(50036, 14)
  "PtrCastToCrossWorkgroupALTERA\0" // IR(50050, 30)
  "PtrCastToCrossWorkgroupINTEL\0" // IR(50080, 29)
  "CrossWorkgroupCastToPtrALTERA\0" // IR(50109, 30)
  "CrossWorkgroupCastToPtrINTEL\0" // IR(50139, 29)
  "ReadPipeBlockingALTERA\0" // IR(50168, 23)
  "ReadPipeBlockingINTEL\0" // IR(50191, 22)
  "WritePipeBlockingALTERA\0" // IR(50213, 24)
  "WritePipeBlockingINTEL\0" // IR(50237, 23)
  "RayQueryGetRayTMinKHR\0" // IR(50260, 22)
  "RayQueryGetRayFlagsKHR\0" // IR(50282, 23)
  "RayQueryGetIntersectionTKHR\0" // IR(50305, 28)
  "RayQueryGetIntersectionInstanceCustomIndexKHR\0" // IR(50333, 46)
  "RayQueryGetIntersectionInstanceIdKHR\0" // IR(50379, 37)
  "RayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR\0" // IR(50416, 65)
  "RayQueryGetIntersectionGeometryIndexKHR\0" // IR(50481, 40)
  "RayQueryGetIntersectionPrimitiveIndexKHR\0" // IR(50521, 41)
  "RayQueryGetIntersectionBarycentricsKHR\0" // IR(50562, 39)
  "RayQueryGetIntersectionFrontFaceKHR\0" // IR(50601, 36)
  "RayQueryGetIntersectionCandidateAABBOpaqueKHR\0" // IR(50637, 46)
  "RayQueryGetIntersectionObjectRayDirectionKHR\0" // IR(50683, 45)
  "RayQueryGetIntersectionObjectRayOriginKHR\0" // IR(50728, 42)
  "RayQueryGetWorldRayDirectionKHR\0" // IR(50770, 32)
  "RayQueryGetWorldRayOriginKHR\0" // IR(50802, 29)
  "RayQueryGetIntersectionObjectToWorldKHR\0" // IR(50831, 40)
  "RayQueryGetIntersectionWorldToObjectKHR\0" // IR(50871, 40)
  "AtomicFAddEXT\0" // IR(50911, 14)
  "TypeBufferSurfaceINTEL\0" // IR(50925, 23)
  "TypeStructContinuedINTEL\0" // IR(50948, 25)
  "ConstantCompositeContinuedINTEL\0" // IR(50973, 32)
  "SpecConstantCompositeContinuedINTEL\0" // IR(51005, 36)
  "CompositeConstructContinuedINTEL\0" // IR(51041, 33)
  "ConvertFToBF16INTEL\0" // IR(51074, 20)
  "ConvertBF16ToFINTEL\0" // IR(51094, 20)
  "ControlBarrierArriveEXT\0" // IR(51114, 24)
  "ControlBarrierArriveINTEL\0" // IR(51138, 26)
  "ControlBarrierWaitEXT\0" // IR(51164, 22)
  "ControlBarrierWaitINTEL\0" // IR(51186, 24)
  "TaskSequenceCreateALTERA\0" // IR(51210, 25)
  "TaskSequenceCreateINTEL\0" // IR(51235, 24)
  "TaskSequenceAsyncALTERA\0" // IR(51259, 24)
  "TaskSequenceAsyncINTEL\0" // IR(51283, 23)
  "TaskSequenceGetALTERA\0" // IR(51306, 22)
  "TaskSequenceGetINTEL\0" // IR(51328, 21)
  "TaskSequenceReleaseALTERA\0" // IR(51349, 26)
  "TaskSequenceReleaseINTEL\0" // IR(51375, 25)
  "TypeTaskSequenceALTERA\0" // IR(51400, 23)
  "TypeTaskSequenceINTEL\0" // IR(51423, 22)
  "SubgroupBlockPrefetchINTEL\0" // IR(51445, 27)
  "Subgroup2DBlockLoadINTEL\0" // IR(51472, 25)
  "Subgroup2DBlockLoadTransformINTEL\0" // IR(51497, 34)
  "Subgroup2DBlockLoadTransposeINTEL\0" // IR(51531, 34)
  "Subgroup2DBlockPrefetchINTEL\0" // IR(51565, 29)
  "Subgroup2DBlockStoreINTEL\0" // IR(51594, 26)
  "SPV_OPERAND_TYPE_OPTIONAL_MATRIX_MULTIPLY_ACCUMULATE_OPERANDS\0" // IR(51620, 62)
  "BitwiseFunctionINTEL\0" // IR(51682, 21)
  "ConditionalExtensionINTEL\0" // IR(51703, 26)
  "ConditionalEntryPointINTEL\0" // IR(51729, 27)
  "ConditionalCapabilityINTEL\0" // IR(51756, 27)
  "SpecConstantTargetINTEL\0" // IR(51783, 24)
  "SpecConstantArchitectureINTEL\0" // IR(51807, 30)
  "SPV_OPERAND_TYPE_VARIABLE_CAPABILITY\0" // IR(51837, 37)
  "SpecConstantCapabilitiesINTEL\0" // IR(51874, 30)
  "ConditionalCopyObjectINTEL\0" // IR(51904, 27)
  "PredicatedLoadINTEL\0" // IR(51931, 20)
  "PredicatedStoreINTEL\0" // IR(51951, 21)
  "GroupIMulKHR\0" // IR(51972, 13)
  "GroupFMulKHR\0" // IR(51985, 13)
  "GroupBitwiseAndKHR\0" // IR(51998, 19)
  "GroupBitwiseOrKHR\0" // IR(52017, 18)
  "GroupBitwiseXorKHR\0" // IR(52035, 19)
  "GroupLogicalAndKHR\0" // IR(52054, 19)
  "GroupLogicalOrKHR\0" // IR(52073, 18)
  "GroupLogicalXorKHR\0" // IR(52091, 19)
  "RoundFToTF32INTEL\0" // IR(52110, 18)
  "MaskedGatherINTEL\0" // IR(52128, 18)
  "MaskedScatterINTEL\0" // IR(52146, 19)
  "ConvertHandleToImageINTEL\0" // IR(52165, 26)
  "ConvertHandleToSamplerINTEL\0" // IR(52191, 28)
  "ConvertHandleToSampledImageINTEL\0" // IR(52219, 33)
  "FDot2MixAcc32VALVE\0" // IR(52252, 19)
  "FDot2MixAcc16VALVE\0" // IR(52271, 19)
  "FDot4MixAcc32VALVE\0" // IR(52290, 19)
  "CALL\0" // IR(52309, 5)
  "MIN_SAD\0" // IR(52314, 8)
  "MIN_SAD_COST\0" // IR(52322, 13)
  "RAW_SAD\0" // IR(52335, 8)
  "DebugInfoNone\0" // IR(52343, 14)
  "DebugCompilationUnit\0" // IR(52357, 21)
  "SPV_OPERAND_TYPE_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(52378, 52)
  "DebugTypeBasic\0" // IR(52430, 15)
  "SPV_OPERAND_TYPE_DEBUG_INFO_FLAGS\0" // IR(52445, 34)
  "DebugTypePointer\0" // IR(52479, 17)
  "SPV_OPERAND_TYPE_DEBUG_TYPE_QUALIFIER\0" // IR(52496, 38)
  "DebugTypeQualifier\0" // IR(52534, 19)
  "DebugTypeArray\0" // IR(52553, 15)
  "DebugTypeVector\0" // IR(52568, 16)
  "DebugTypedef\0" // IR(52584, 13)
  "DebugTypeFunction\0" // IR(52597, 18)
  "DebugTypeEnum\0" // IR(52615, 14)
  "SPV_OPERAND_TYPE_DEBUG_COMPOSITE_TYPE\0" // IR(52629, 38)
  "DebugTypeComposite\0" // IR(52667, 19)
  "DebugTypeMember\0" // IR(52686, 16)
  "DebugTypeInheritance\0" // IR(52702, 21)
  "DebugTypePtrToMember\0" // IR(52723, 21)
  "DebugTypeTemplate\0" // IR(52744, 18)
  "DebugTypeTemplateParameter\0" // IR(52762, 27)
  "DebugTypeTemplateTemplateParameter\0" // IR(52789, 35)
  "DebugTypeTemplateParameterPack\0" // IR(52824, 31)
  "DebugGlobalVariable\0" // IR(52855, 20)
  "DebugFunctionDeclaration\0" // IR(52875, 25)
  "DebugFunction\0" // IR(52900, 14)
  "DebugLexicalBlock\0" // IR(52914, 18)
  "DebugLexicalBlockDiscriminator\0" // IR(52932, 31)
  "DebugScope\0" // IR(52963, 11)
  "DebugNoScope\0" // IR(52974, 13)
  "DebugInlinedAt\0" // IR(52987, 15)
  "SPV_OPERAND_TYPE_OPTIONAL_LITERAL_INTEGER\0" // IR(53002, 42)
  "DebugLocalVariable\0" // IR(53044, 19)
  "DebugInlinedVariable\0" // IR(53063, 21)
  "DebugDeclare\0" // IR(53084, 13)
  "DebugValue\0" // IR(53097, 11)
  "SPV_OPERAND_TYPE_DEBUG_OPERATION\0" // IR(53108, 33)
  "DebugOperation\0" // IR(53141, 15)
  "DebugExpression\0" // IR(53156, 16)
  "DebugMacroDef\0" // IR(53172, 14)
  "DebugMacroUndef\0" // IR(53186, 16)
  "Round\0" // IR(53202, 6)
  "RoundEven\0" // IR(53208, 10)
  "Trunc\0" // IR(53218, 6)
  "FAbs\0" // IR(53224, 5)
  "SAbs\0" // IR(53229, 5)
  "FSign\0" // IR(53234, 6)
  "SSign\0" // IR(53240, 6)
  "Floor\0" // IR(53246, 6)
  "Ceil\0" // IR(53252, 5)
  "Fract\0" // IR(53257, 6)
  "Radians\0" // IR(53263, 8)
  "Degrees\0" // IR(53271, 8)
  "Sin\0" // IR(53279, 4)
  "Cos\0" // IR(53283, 4)
  "Tan\0" // IR(53287, 4)
  "Asin\0" // IR(53291, 5)
  "Acos\0" // IR(53296, 5)
  "Atan\0" // IR(53301, 5)
  "Sinh\0" // IR(53306, 5)
  "Cosh\0" // IR(53311, 5)
  "Tanh\0" // IR(53316, 5)
  "Asinh\0" // IR(53321, 6)
  "Acosh\0" // IR(53327, 6)
  "Atanh\0" // IR(53333, 6)
  "Atan2\0" // IR(53339, 6)
  "Pow\0" // IR(53345, 4)
  "Exp\0" // IR(53349, 4)
  "Log\0" // IR(53353, 4)
  "Exp2\0" // IR(53357, 5)
  "Log2\0" // IR(53362, 5)
  "Sqrt\0" // IR(53367, 5)
  "InverseSqrt\0" // IR(53372, 12)
  "Determinant\0" // IR(53384, 12)
  "MatrixInverse\0" // IR(53396, 14)
  "Modf\0" // IR(53410, 5)
  "ModfStruct\0" // IR(53415, 11)
  "FMin\0" // IR(53426, 5)
  "UMin\0" // IR(53431, 5)
  "SMin\0" // IR(53436, 5)
  "FMax\0" // IR(53441, 5)
  "UMax\0" // IR(53446, 5)
  "SMax\0" // IR(53451, 5)
  "FClamp\0" // IR(53456, 7)
  "UClamp\0" // IR(53463, 7)
  "SClamp\0" // IR(53470, 7)
  "FMix\0" // IR(53477, 5)
  "IMix\0" // IR(53482, 5)
  "Step\0" // IR(53487, 5)
  "SmoothStep\0" // IR(53492, 11)
  "Fma\0" // IR(53503, 4)
  "Frexp\0" // IR(53507, 6)
  "FrexpStruct\0" // IR(53513, 12)
  "Ldexp\0" // IR(53525, 6)
  "PackSnorm4x8\0" // IR(53531, 13)
  "PackUnorm4x8\0" // IR(53544, 13)
  "PackSnorm2x16\0" // IR(53557, 14)
  "PackUnorm2x16\0" // IR(53571, 14)
  "PackHalf2x16\0" // IR(53585, 13)
  "PackDouble2x32\0" // IR(53598, 15)
  "UnpackSnorm2x16\0" // IR(53613, 16)
  "UnpackUnorm2x16\0" // IR(53629, 16)
  "UnpackHalf2x16\0" // IR(53645, 15)
  "UnpackSnorm4x8\0" // IR(53660, 15)
  "UnpackUnorm4x8\0" // IR(53675, 15)
  "UnpackDouble2x32\0" // IR(53690, 17)
  "Length\0" // IR(53707, 7)
  "Distance\0" // IR(53714, 9)
  "Cross\0" // IR(53723, 6)
  "Normalize\0" // IR(53729, 10)
  "FaceForward\0" // IR(53739, 12)
  "Reflect\0" // IR(53751, 8)
  "Refract\0" // IR(53759, 8)
  "FindILsb\0" // IR(53767, 9)
  "FindSMsb\0" // IR(53776, 9)
  "FindUMsb\0" // IR(53785, 9)
  "InterpolateAtCentroid\0" // IR(53794, 22)
  "InterpolateAtSample\0" // IR(53816, 20)
  "InterpolateAtOffset\0" // IR(53836, 20)
  "NMin\0" // IR(53856, 5)
  "NMax\0" // IR(53861, 5)
  "NClamp\0" // IR(53866, 7)
  "ArgumentInfo\0" // IR(53873, 13)
  "ArgumentStorageBuffer\0" // IR(53886, 22)
  "ArgumentUniform\0" // IR(53908, 16)
  "ArgumentPodStorageBuffer\0" // IR(53924, 25)
  "ArgumentPodUniform\0" // IR(53949, 19)
  "ArgumentPodPushConstant\0" // IR(53968, 24)
  "ArgumentSampledImage\0" // IR(53992, 21)
  "ArgumentStorageImage\0" // IR(54013, 21)
  "ArgumentSampler\0" // IR(54034, 16)
  "ArgumentWorkgroup\0" // IR(54050, 18)
  "SpecConstantWorkgroupSize\0" // IR(54068, 26)
  "SpecConstantGlobalOffset\0" // IR(54094, 25)
  "SpecConstantWorkDim\0" // IR(54119, 20)
  "PushConstantGlobalOffset\0" // IR(54139, 25)
  "PushConstantEnqueuedLocalSize\0" // IR(54164, 30)
  "PushConstantGlobalSize\0" // IR(54194, 23)
  "PushConstantRegionOffset\0" // IR(54217, 25)
  "PushConstantNumWorkgroups\0" // IR(54242, 26)
  "PushConstantRegionGroupOffset\0" // IR(54268, 30)
  "ConstantDataStorageBuffer\0" // IR(54298, 26)
  "ConstantDataUniform\0" // IR(54324, 20)
  "PropertyRequiredWorkgroupSize\0" // IR(54344, 30)
  "SpecConstantSubgroupMaxSize\0" // IR(54374, 28)
  "ArgumentPointerPushConstant\0" // IR(54402, 28)
  "ArgumentPointerUniform\0" // IR(54430, 23)
  "ProgramScopeVariablesStorageBuffer\0" // IR(54453, 35)
  "ProgramScopeVariablePointerRelocation\0" // IR(54488, 38)
  "ImageArgumentInfoChannelOrderPushConstant\0" // IR(54526, 42)
  "ImageArgumentInfoChannelDataTypePushConstant\0" // IR(54568, 45)
  "ImageArgumentInfoChannelOrderUniform\0" // IR(54613, 37)
  "ImageArgumentInfoChannelDataTypeUniform\0" // IR(54650, 40)
  "ArgumentStorageTexelBuffer\0" // IR(54690, 27)
  "ArgumentUniformTexelBuffer\0" // IR(54717, 27)
  "ConstantDataPointerPushConstant\0" // IR(54744, 32)
  "ProgramScopeVariablePointerPushConstant\0" // IR(54776, 40)
  "PrintfInfo\0" // IR(54816, 11)
  "PrintfBufferStorageBuffer\0" // IR(54827, 26)
  "PrintfBufferPointerPushConstant\0" // IR(54853, 32)
  "NormalizedSamplerMaskPushConstant\0" // IR(54885, 34)
  "WorkgroupVariableSize\0" // IR(54919, 22)
  "DebugPrintf\0" // IR(54941, 12)
  "DebugGraph\0" // IR(54953, 11)
  "DebugTensor\0" // IR(54964, 12)
  "DebugImportedEntity\0" // IR(54976, 20)
  "DebugSource\0" // IR(54996, 12)
  "DebugFunctionDefinition\0" // IR(55008, 24)
  "DebugSourceContinued\0" // IR(55032, 21)
  "DebugLine\0" // IR(55053, 10)
  "DebugNoLine\0" // IR(55063, 12)
  "DebugBuildIdentifier\0" // IR(55075, 21)
  "DebugStoragePath\0" // IR(55096, 17)
  "DebugEntryPoint\0" // IR(55113, 16)
  "DebugTypeMatrix\0" // IR(55129, 16)
  "DebugTypeVectorIdEXT\0" // IR(55145, 21)
  "DebugTypeCooperativeMatrixKHR\0" // IR(55166, 30)
  "Configuration\0" // IR(55196, 14)
  "StartCounter\0" // IR(55210, 13)
  "StopCounter\0" // IR(55223, 12)
  "PushConstants\0" // IR(55235, 14)
  "SpecializationMapEntry\0" // IR(55249, 23)
  "DescriptorSetBuffer\0" // IR(55272, 20)
  "DescriptorSetImage\0" // IR(55292, 19)
  "DescriptorSetSampler\0" // IR(55311, 21)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_BASE_TYPE_ATTRIBUTE_ENCODING\0" // IR(55332, 63)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_INFO_FLAGS\0" // IR(55395, 45)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_TYPE_QUALIFIER\0" // IR(55440, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_COMPOSITE_TYPE\0" // IR(55489, 49)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_OPERATION\0" // IR(55538, 44)
  "SPV_OPERAND_TYPE_CLDEBUG100_DEBUG_IMPORTED_ENTITY\0" // IR(55582, 50)
  "DebugModuleINTEL\0" // IR(55632, 17)
  "acos\0" // IR(55649, 5)
  "acosh\0" // IR(55654, 6)
  "acospi\0" // IR(55660, 7)
  "asin\0" // IR(55667, 5)
  "asinh\0" // IR(55672, 6)
  "asinpi\0" // IR(55678, 7)
  "atan\0" // IR(55685, 5)
  "atan2\0" // IR(55690, 6)
  "atanh\0" // IR(55696, 6)
  "atanpi\0" // IR(55702, 7)
  "atan2pi\0" // IR(55709, 8)
  "cbrt\0" // IR(55717, 5)
  "ceil\0" // IR(55722, 5)
  "copysign\0" // IR(55727, 9)
  "cos\0" // IR(55736, 4)
  "cosh\0" // IR(55740, 5)
  "cospi\0" // IR(55745, 6)
  "erfc\0" // IR(55751, 5)
  "erf\0" // IR(55756, 4)
  "exp\0" // IR(55760, 4)
  "exp2\0" // IR(55764, 5)
  "exp10\0" // IR(55769, 6)
  "expm1\0" // IR(55775, 6)
  "fabs\0" // IR(55781, 5)
  "fdim\0" // IR(55786, 5)
  "floor\0" // IR(55791, 6)
  "fma\0" // IR(55797, 4)
  "fmax\0" // IR(55801, 5)
  "fmin\0" // IR(55806, 5)
  "fmod\0" // IR(55811, 5)
  "fract\0" // IR(55816, 6)
  "frexp\0" // IR(55822, 6)
  "hypot\0" // IR(55828, 6)
  "ilogb\0" // IR(55834, 6)
  "ldexp\0" // IR(55840, 6)
  "lgamma\0" // IR(55846, 7)
  "lgamma_r\0" // IR(55853, 9)
  "log\0" // IR(55862, 4)
  "log2\0" // IR(55866, 5)
  "log10\0" // IR(55871, 6)
  "log1p\0" // IR(55877, 6)
  "logb\0" // IR(55883, 5)
  "mad\0" // IR(55888, 4)
  "maxmag\0" // IR(55892, 7)
  "minmag\0" // IR(55899, 7)
  "modf\0" // IR(55906, 5)
  "nan\0" // IR(55911, 4)
  "nextafter\0" // IR(55915, 10)
  "pow\0" // IR(55925, 4)
  "pown\0" // IR(55929, 5)
  "powr\0" // IR(55934, 5)
  "remainder\0" // IR(55939, 10)
  "remquo\0" // IR(55949, 7)
  "rint\0" // IR(55956, 5)
  "rootn\0" // IR(55961, 6)
  "round\0" // IR(55967, 6)
  "rsqrt\0" // IR(55973, 6)
  "sin\0" // IR(55979, 4)
  "sincos\0" // IR(55983, 7)
  "sinh\0" // IR(55990, 5)
  "sinpi\0" // IR(55995, 6)
  "sqrt\0" // IR(56001, 5)
  "tan\0" // IR(56006, 4)
  "tanh\0" // IR(56010, 5)
  "tanpi\0" // IR(56015, 6)
  "tgamma\0" // IR(56021, 7)
  "trunc\0" // IR(56028, 6)
  "half_cos\0" // IR(56034, 9)
  "half_divide\0" // IR(56043, 12)
  "half_exp\0" // IR(56055, 9)
  "half_exp2\0" // IR(56064, 10)
  "half_exp10\0" // IR(56074, 11)
  "half_log\0" // IR(56085, 9)
  "half_log2\0" // IR(56094, 10)
  "half_log10\0" // IR(56104, 11)
  "half_powr\0" // IR(56115, 10)
  "half_recip\0" // IR(56125, 11)
  "half_rsqrt\0" // IR(56136, 11)
  "half_sin\0" // IR(56147, 9)
  "half_sqrt\0" // IR(56156, 10)
  "half_tan\0" // IR(56166, 9)
  "native_cos\0" // IR(56175, 11)
  "native_divide\0" // IR(56186, 14)
  "native_exp\0" // IR(56200, 11)
  "native_exp2\0" // IR(56211, 12)
  "native_exp10\0" // IR(56223, 13)
  "native_log\0" // IR(56236, 11)
  "native_log2\0" // IR(56247, 12)
  "native_log10\0" // IR(56259, 13)
  "native_powr\0" // IR(56272, 12)
  "native_recip\0" // IR(56284, 13)
  "native_rsqrt\0" // IR(56297, 13)
  "native_sin\0" // IR(56310, 11)
  "native_sqrt\0" // IR(56321, 12)
  "native_tan\0" // IR(56333, 11)
  "fclamp\0" // IR(56344, 7)
  "degrees\0" // IR(56351, 8)
  "fmax_common\0" // IR(56359, 12)
  "fmin_common\0" // IR(56371, 12)
  "mix\0" // IR(56383, 4)
  "radians\0" // IR(56387, 8)
  "step\0" // IR(56395, 5)
  "smoothstep\0" // IR(56400, 11)
  "sign\0" // IR(56411, 5)
  "cross\0" // IR(56416, 6)
  "distance\0" // IR(56422, 9)
  "length\0" // IR(56431, 7)
  "normalize\0" // IR(56438, 10)
  "fast_distance\0" // IR(56448, 14)
  "fast_length\0" // IR(56462, 12)
  "fast_normalize\0" // IR(56474, 15)
  "s_abs\0" // IR(56489, 6)
  "s_abs_diff\0" // IR(56495, 11)
  "s_add_sat\0" // IR(56506, 10)
  "u_add_sat\0" // IR(56516, 10)
  "s_hadd\0" // IR(56526, 7)
  "u_hadd\0" // IR(56533, 7)
  "s_rhadd\0" // IR(56540, 8)
  "u_rhadd\0" // IR(56548, 8)
  "s_clamp\0" // IR(56556, 8)
  "u_clamp\0" // IR(56564, 8)
  "clz\0" // IR(56572, 4)
  "ctz\0" // IR(56576, 4)
  "s_mad_hi\0" // IR(56580, 9)
  "u_mad_sat\0" // IR(56589, 10)
  "s_mad_sat\0" // IR(56599, 10)
  "s_max\0" // IR(56609, 6)
  "u_max\0" // IR(56615, 6)
  "s_min\0" // IR(56621, 6)
  "u_min\0" // IR(56627, 6)
  "s_mul_hi\0" // IR(56633, 9)
  "rotate\0" // IR(56642, 7)
  "s_sub_sat\0" // IR(56649, 10)
  "u_sub_sat\0" // IR(56659, 10)
  "u_upsample\0" // IR(56669, 11)
  "s_upsample\0" // IR(56680, 11)
  "popcount\0" // IR(56691, 9)
  "s_mad24\0" // IR(56700, 8)
  "u_mad24\0" // IR(56708, 8)
  "s_mul24\0" // IR(56716, 8)
  "u_mul24\0" // IR(56724, 8)
  "vloadn\0" // IR(56732, 7)
  "vstoren\0" // IR(56739, 8)
  "vload_half\0" // IR(56747, 11)
  "vload_halfn\0" // IR(56758, 12)
  "vstore_half\0" // IR(56770, 12)
  "vstore_half_r\0" // IR(56782, 14)
  "vstore_halfn\0" // IR(56796, 13)
  "vstore_halfn_r\0" // IR(56809, 15)
  "vloada_halfn\0" // IR(56824, 13)
  "vstorea_halfn\0" // IR(56837, 14)
  "vstorea_halfn_r\0" // IR(56851, 16)
  "shuffle\0" // IR(56867, 8)
  "shuffle2\0" // IR(56875, 9)
  "printf\0" // IR(56884, 7)
  "prefetch\0" // IR(56891, 9)
  "bitselect\0" // IR(56900, 10)
  "select\0" // IR(56910, 7)
  "u_abs\0" // IR(56917, 6)
  "u_abs_diff\0" // IR(56923, 11)
  "u_mul_hi\0" // IR(56934, 9)
  "u_mad_hi\0" // IR(56943, 9)
  "CubeFaceIndexAMD\0" // IR(56952, 17)
  "CubeFaceCoordAMD\0" // IR(56969, 17)
  "TimeAMD\0" // IR(56986, 8)
  "SwizzleInvocationsAMD\0" // IR(56994, 22)
  "SwizzleInvocationsMaskedAMD\0" // IR(57016, 28)
  "WriteInvocationAMD\0" // IR(57044, 19)
  "MbcntAMD\0" // IR(57063, 9)
  "InterpolateAtVertexAMD\0" // IR(57072, 23)
  "FMin3AMD\0" // IR(57095, 9)
  "UMin3AMD\0" // IR(57104, 9)
  "SMin3AMD\0" // IR(57113, 9)
  "FMax3AMD\0" // IR(57122, 9)
  "UMax3AMD\0" // IR(57131, 9)
  "SMax3AMD\0" // IR(57140, 9)
  "FMid3AMD\0" // IR(57149, 9)
  "UMid3AMD\0" // IR(57158, 9)
  "SMid3AMD\0" // IR(57167, 9)
  "ARGMAX\0" // IR(57176, 7)
  "AVG_POOL2D\0" // IR(57183, 11)
  "CONV2D\0" // IR(57194, 7)
  "CONV3D\0" // IR(57201, 7)
  "DEPTHWISE_CONV2D\0" // IR(57208, 17)
  "FFT2D\0" // IR(57225, 6)
  "MATMUL\0" // IR(57231, 7)
  "MAX_POOL2D\0" // IR(57238, 11)
  "RFFT2D\0" // IR(57249, 7)
  "TRANSPOSE_CONV2D\0" // IR(57256, 17)
  "CLAMP\0" // IR(57273, 6)
  "ERF\0" // IR(57279, 4)
  "SIGMOID\0" // IR(57283, 8)
  "TANH\0" // IR(57291, 5)
  "ADD\0" // IR(57296, 4)
  "ARITHMETIC_RIGHT_SHIFT\0" // IR(57300, 23)
  "BITWISE_AND\0" // IR(57323, 12)
  "BITWISE_OR\0" // IR(57335, 11)
  "BITWISE_XOR\0" // IR(57346, 12)
  "INTDIV\0" // IR(57358, 7)
  "LOGICAL_AND\0" // IR(57365, 12)
  "LOGICAL_LEFT_SHIFT\0" // IR(57377, 19)
  "LOGICAL_RIGHT_SHIFT\0" // IR(57396, 20)
  "LOGICAL_OR\0" // IR(57416, 11)
  "LOGICAL_XOR\0" // IR(57427, 12)
  "MAXIMUM\0" // IR(57439, 8)
  "MINIMUM\0" // IR(57447, 8)
  "MUL\0" // IR(57455, 4)
  "POW\0" // IR(57459, 4)
  "SUB\0" // IR(57463, 4)
  "TABLE\0" // IR(57467, 6)
  "ABS\0" // IR(57473, 4)
  "BITWISE_NOT\0" // IR(57477, 12)
  "CEIL\0" // IR(57489, 5)
  "CLZ\0" // IR(57494, 4)
  "COS\0" // IR(57498, 4)
  "EXP\0" // IR(57502, 4)
  "FLOOR\0" // IR(57506, 6)
  "LOG\0" // IR(57512, 4)
  "LOGICAL_NOT\0" // IR(57516, 12)
  "NEGATE\0" // IR(57528, 7)
  "RECIPROCAL\0" // IR(57535, 11)
  "RSQRT\0" // IR(57546, 6)
  "SIN\0" // IR(57552, 4)
  "SELECT\0" // IR(57556, 7)
  "EQUAL\0" // IR(57563, 6)
  "GREATER\0" // IR(57569, 8)
  "GREATER_EQUAL\0" // IR(57577, 14)
  "REDUCE_ALL\0" // IR(57591, 11)
  "REDUCE_ANY\0" // IR(57602, 11)
  "REDUCE_MAX\0" // IR(57613, 11)
  "REDUCE_MIN\0" // IR(57624, 11)
  "REDUCE_PRODUCT\0" // IR(57635, 15)
  "REDUCE_SUM\0" // IR(57650, 11)
  "CONCAT\0" // IR(57661, 7)
  "PAD\0" // IR(57668, 4)
  "RESHAPE\0" // IR(57672, 8)
  "REVERSE\0" // IR(57680, 8)
  "SLICE\0" // IR(57688, 6)
  "TILE\0" // IR(57694, 5)
  "TRANSPOSE\0" // IR(57699, 10)
  "GATHER\0" // IR(57709, 7)
  "SCATTER\0" // IR(57716, 8)
  "RESIZE\0" // IR(57724, 7)
  "CAST\0" // IR(57731, 5)
  "RESCALE\0" // IR(57736, 8)
;

// Array of IndexRanges, where each represents a string by referencing
// the kStrings table.
// This array contains all sequences of alias strings used in the grammar.
// This table is referenced by an IndexRange elsewhere, i.e. by the 'aliases'
// field of an instruction or operand description.
static const IndexRange kAliasSpans[] = {
  IR(6031, 27), // 0 PhysicalStorageBuffer64EXT
  IR(6942, 18), // 1 SubgroupEqMaskKHR
  IR(6997, 18), // 2 SubgroupGeMaskKHR
  IR(7030, 18), // 3 SubgroupGtMaskKHR
  IR(7063, 18), // 4 SubgroupLeMaskKHR
  IR(7096, 18), // 5 SubgroupLtMaskKHR
  IR(8003, 12), // 6 BaryCoordNV
  IR(8058, 19), // 7 BaryCoordNoPerspNV
  IR(8089, 15), // 8 FragmentSizeNV
  IR(8146, 22), // 9 InvocationsPerPixelNV
  IR(8274, 11), // 10 LaunchIdNV
  IR(8299, 13), // 11 LaunchSizeNV
  IR(8330, 17), // 12 WorldRayOriginNV
  IR(8368, 20), // 13 WorldRayDirectionNV
  IR(8407, 18), // 14 ObjectRayOriginNV
  IR(8447, 21), // 15 ObjectRayDirectionNV
  IR(8479, 10), // 16 RayTminNV
  IR(8500, 10), // 17 RayTmaxNV
  IR(8533, 22), // 18 InstanceCustomIndexNV
  IR(8572, 16), // 19 ObjectToWorldNV
  IR(8605, 16), // 20 WorldToObjectNV
  IR(8639, 10), // 21 HitKindNV
  IR(8870, 19), // 22 IncomingRayFlagsNV
  IR(10759, 28), // 23 StorageUniformBufferBlock16
  IR(10822, 17), // 24 StorageUniform16
  IR(11803, 27), // 25 ShaderViewportIndexLayerNV
  IR(11847, 22), // 26 FragmentBarycentricNV
  IR(11900, 30), // 27 ComputeDerivativeGroupQuadsNV
  IR(11930, 14), // 28 ShadingRateNV
  IR(11974, 29), // 29 GroupNonUniformPartitionedNV
  IR(12020, 20), // 30 ShaderNonUniformEXT
  IR(12063, 26), // 31 RuntimeDescriptorArrayEXT
  IR(12125, 39), // 32 InputAttachmentArrayDynamicIndexingEXT
  IR(12203, 42), // 33 UniformTexelBufferArrayDynamicIndexingEXT
  IR(12284, 42), // 34 StorageTexelBufferArrayDynamicIndexingEXT
  IR(12363, 40), // 35 UniformBufferArrayNonUniformIndexingEXT
  IR(12439, 39), // 36 SampledImageArrayNonUniformIndexingEXT
  IR(12515, 40), // 37 StorageBufferArrayNonUniformIndexingEXT
  IR(12591, 39), // 38 StorageImageArrayNonUniformIndexingEXT
  IR(12669, 42), // 39 InputAttachmentArrayNonUniformIndexingEXT
  IR(12753, 45), // 40 UniformTexelBufferArrayNonUniformIndexingEXT
  IR(12840, 45), // 41 StorageTexelBufferArrayNonUniformIndexingEXT
  IR(12903, 21), // 42 VulkanMemoryModelKHR
  IR(12953, 32), // 43 VulkanMemoryModelDeviceScopeKHR
  IR(12985, 34), // 44 PhysicalStorageBufferAddressesEXT
  IR(13051, 31), // 45 ComputeDerivativeGroupLinearNV
  IR(13255, 28), // 46 DemoteToHelperInvocationEXT
  IR(13335, 29), // 47 RayTracingOpacityMicromapEXT
  IR(13640, 30), // 48 CooperativeMatrixReductionsNV
  IR(13742, 40), // 49 CooperativeMatrixPerElementOperationsNV
  IR(14495, 26), // 50 FPGAMemoryAttributesINTEL
  IR(14574, 32), // 51 ArbitraryPrecisionIntegersINTEL
  IR(14644, 37), // 52 ArbitraryPrecisionFloatingPointINTEL
  IR(14734, 22), // 53 FPGALoopControlsINTEL
  IR(14829, 24), // 54 FPGAMemoryAccessesINTEL
  IR(14881, 27), // 55 FPGAClusterAttributesINTEL
  IR(14923, 14), // 56 LoopFuseINTEL
  IR(14958, 20), // 57 FPGADSPControlINTEL
  IR(15045, 40), // 58 FPGAInvocationPipeliningAttributesINTEL
  IR(15110, 24), // 59 FPGABufferLocationINTEL
  IR(15169, 34), // 60 ArbitraryPrecisionFixedPointINTEL
  IR(15227, 23), // 61 USMStorageClassesINTEL
  IR(15280, 29), // 62 RuntimeAlignedAttributeINTEL
  IR(15323, 13), // 63 IOPipesINTEL
  IR(15356, 19), // 64 BlockingPipesINTEL
  IR(15389, 13), // 65 FPGARegINTEL
  IR(15421, 22), // 66 DotProductInputAllKHR
  IR(15465, 25), // 67 DotProductInput4x8BitKHR
  IR(15518, 31), // 68 DotProductInput4x8BitPackedKHR
  IR(15560, 14), // 69 DotProductKHR
  IR(15774, 13), // 70 OptNoneINTEL
  IR(15868, 18), // 71 SplitBarrierINTEL
  IR(15935, 29), // 72 FPGAClusterAttributesV2INTEL
  IR(16011, 18), // 73 TaskSequenceINTEL
  IR(16070, 24), // 74 FPGALatencyControlINTEL
  IR(16123, 28), // 75 FPGAArgumentInterfacesINTEL
  IR(16217, 35), // 76 GlobalVariableFPGADecorationsINTEL
  IR(19226, 15), // 77 PerPrimitiveNV
  IR(19274, 12), // 78 PerVertexNV
  IR(19297, 14), // 79 NonUniformEXT
  IR(19327, 19), // 80 RestrictPointerEXT
  IR(19361, 18), // 81 AliasedPointerEXT
  IR(19754, 24), // 82 HlslCounterBufferGOOGLE
  IR(19791, 19), // 83 HlslSemanticGOOGLE
  IR(19921, 14), // 84 RegisterINTEL
  IR(19948, 12), // 85 MemoryINTEL
  IR(19975, 14), // 86 NumbanksINTEL
  IR(20005, 15), // 87 BankwidthINTEL
  IR(20043, 22), // 88 MaxPrivateCopiesINTEL
  IR(20082, 16), // 89 SinglepumpINTEL
  IR(20115, 16), // 90 DoublepumpINTEL
  IR(20151, 19), // 91 MaxReplicatesINTEL
  IR(20191, 20), // 92 SimpleDualPortINTEL
  IR(20223, 11), // 93 MergeINTEL
  IR(20291, 14), // 94 BankBitsINTEL
  IR(20326, 20), // 95 ForcePow2DepthINTEL
  IR(20363, 16), // 96 StridesizeINTEL
  IR(20394, 14), // 97 WordsizeINTEL
  IR(20427, 18), // 98 TrueDualPortINTEL
  IR(20465, 19), // 99 BurstCoalesceINTEL
  IR(20500, 15), // 100 CacheSizeINTEL
  IR(20544, 28), // 101 DontStaticallyCoalesceINTEL
  IR(20587, 14), // 102 PrefetchINTEL
  IR(20619, 17), // 103 StallEnableINTEL
  IR(20662, 25), // 104 FuseLoopsInFunctionINTEL
  IR(20707, 19), // 105 MathOpDSPModeINTEL
  IR(20780, 24), // 106 InitiationIntervalINTEL
  IR(20825, 20), // 107 MaxConcurrencyINTEL
  IR(20866, 20), // 108 PipelineEnableINTEL
  IR(20907, 20), // 109 BufferLocationINTEL
  IR(20947, 19), // 110 IOPipeStorageINTEL
  IR(21125, 15), // 111 StallFreeINTEL
  IR(21223, 25), // 112 LatencyControlLabelINTEL
  IR(21279, 30), // 113 LatencyControlConstraintINTEL
  IR(21337, 27), // 114 ConduitKernelArgumentINTEL
  IR(21396, 31), // 115 RegisterMapKernelArgumentINTEL
  IR(21461, 33), // 116 MMHostInterfaceAddressWidthINTEL
  IR(21525, 30), // 117 MMHostInterfaceDataWidthINTEL
  IR(21584, 28), // 118 MMHostInterfaceLatencyINTEL
  IR(21681, 34), // 119 MMHostInterfaceReadWriteModeINTEL
  IR(21745, 29), // 120 MMHostInterfaceMaxBurstINTEL
  IR(21807, 32), // 121 MMHostInterfaceWaitRequestINTEL
  IR(21866, 26), // 122 StableKernelArgumentINTEL
  IR(22009, 14), // 123 InitModeINTEL
  IR(22052, 28), // 124 ImplementInRegisterMapINTEL
  IR(23448, 14), // 125 OutputLinesNV
  IR(23482, 19), // 126 OutputPrimitivesNV
  IR(23525, 23), // 127 DerivativeGroupQuadsNV
  IR(23573, 24), // 128 DerivativeGroupLinearNV
  IR(23616, 18), // 129 OutputTrianglesNV
  IR(24385, 16), // 130 RayGenerationNV
  IR(24417, 15), // 131 IntersectionNV
  IR(24442, 9), // 132 AnyHitNV
  IR(24465, 13), // 133 ClosestHitNV
  IR(24486, 7), // 134 MissNV
  IR(24505, 11), // 135 CallableNV
  IR(24692, 23), // 136 AllowContractFastINTEL
  IR(24728, 18), // 137 AllowReassocINTEL
  IR(24954, 20), // 138 RuntimeAlignedINTEL
  IR(25098, 20), // 139 PartitionedReduceNV
  IR(25146, 27), // 140 PartitionedInclusiveScanNV
  IR(25201, 27), // 141 PartitionedExclusiveScanNV
  IR(25332, 22), // 142 MakeTexelAvailableKHR
  IR(25371, 20), // 143 MakeTexelVisibleKHR
  IR(25407, 19), // 144 NonPrivateTexelKHR
  IR(25440, 17), // 145 VolatileTexelKHR
  IR(25999, 27), // 146 InitOnDeviceReprogramINTEL
  IR(26050, 23), // 147 InitOnDeviceResetINTEL
  IR(26392, 21), // 148 DependencyArrayINTEL
  IR(26432, 18), // 149 LoopCoalesceINTEL
  IR(26472, 21), // 150 MaxInterleavingINTEL
  IR(26520, 26), // 151 SpeculatedIterationsINTEL
  IR(26561, 14), // 152 NoFusionINTEL
  IR(26591, 15), // 153 LoopCountINTEL
  IR(26633, 26), // 154 MaxReinvocationDelayINTEL
  IR(27025, 24), // 155 MakePointerAvailableKHR
  IR(27068, 22), // 156 MakePointerVisibleKHR
  IR(27108, 21), // 157 NonPrivatePointerKHR
  IR(27195, 10), // 158 VulkanKHR
  IR(16955, 5), // 159 None
  IR(27378, 16), // 160 OutputMemoryKHR
  IR(27408, 17), // 161 MakeAvailableKHR
  IR(27437, 15), // 162 MakeVisibleKHR
  IR(27513, 28), // 163 PackedVectorFormat4x8BitKHR
  IR(27828, 24), // 164 SkipBuiltinPrimitivesNV
  IR(27895, 30), // 165 ForceOpacityMicromap2StateEXT
  IR(28598, 15), // 166 QueueFamilyKHR
  IR(28983, 15), // 167 CallableDataNV
  IR(29022, 23), // 168 IncomingCallableDataNV
  IR(29059, 13), // 169 RayPayloadNV
  IR(29088, 15), // 170 HitAttributeNV
  IR(29125, 21), // 171 IncomingRayPayloadNV
  IR(29168, 21), // 172 ShaderRecordBufferNV
  IR(29211, 25), // 173 PhysicalStorageBufferEXT
  IR(29337, 16), // 174 DeviceOnlyINTEL
  IR(29368, 14), // 175 HostOnlyINTEL
  IR(36623, 8), // 176 SDotKHR
  IR(36636, 8), // 177 UDotKHR
  IR(36650, 9), // 178 SUDotKHR
  IR(36670, 14), // 179 SDotAccSatKHR
  IR(36695, 14), // 180 UDotAccSatKHR
  IR(36721, 15), // 181 SUDotAccSatKHR
  IR(39038, 24), // 182 TypeCooperativeVectorNV
  IR(39235, 27), // 183 CooperativeMatrixConvertNV
  IR(39325, 27), // 184 GroupNonUniformPartitionNV
  IR(40407, 21), // 185 ReportIntersectionNV
  IR(40582, 28), // 186 TypeAccelerationStructureNV
  IR(40663, 23), // 187 RayQueryGetClusterIdNV
  IR(41124, 26), // 188 CooperativeMatrixReduceNV
  IR(41319, 32), // 189 CooperativeMatrixPerElementOpNV
  IR(42891, 21), // 190 DecorateStringGOOGLE
  IR(42933, 27), // 191 MemberDecorateStringGOOGLE
  IR(48190, 28), // 192 ArbitraryFloatSinCosPiINTEL
  IR(48243, 24), // 193 ArbitraryFloatCastINTEL
  IR(48299, 31), // 194 ArbitraryFloatCastFromIntINTEL
  IR(48360, 29), // 195 ArbitraryFloatCastToIntINTEL
  IR(48413, 23), // 196 ArbitraryFloatAddINTEL
  IR(48460, 23), // 197 ArbitraryFloatSubINTEL
  IR(48507, 23), // 198 ArbitraryFloatMulINTEL
  IR(48554, 23), // 199 ArbitraryFloatDivINTEL
  IR(48600, 22), // 200 ArbitraryFloatGTINTEL
  IR(48645, 22), // 201 ArbitraryFloatGEINTEL
  IR(48690, 22), // 202 ArbitraryFloatLTINTEL
  IR(48735, 22), // 203 ArbitraryFloatLEINTEL
  IR(48780, 22), // 204 ArbitraryFloatEQINTEL
  IR(48828, 25), // 205 ArbitraryFloatRecipINTEL
  IR(48879, 25), // 206 ArbitraryFloatRSqrtINTEL
  IR(48929, 24), // 207 ArbitraryFloatCbrtINTEL
  IR(48979, 25), // 208 ArbitraryFloatHypotINTEL
  IR(49029, 24), // 209 ArbitraryFloatSqrtINTEL
  IR(49713, 15), // 210 FixedSqrtINTEL
  IR(49745, 16), // 211 FixedRecipINTEL
  IR(49778, 16), // 212 FixedRsqrtINTEL
  IR(49809, 14), // 213 FixedSinINTEL
  IR(49838, 14), // 214 FixedCosINTEL
  IR(49870, 17), // 215 FixedSinCosINTEL
  IR(49904, 16), // 216 FixedSinPiINTEL
  IR(49937, 16), // 217 FixedCosPiINTEL
  IR(49973, 19), // 218 FixedSinCosPiINTEL
  IR(50007, 14), // 219 FixedLogINTEL
  IR(50036, 14), // 220 FixedExpINTEL
  IR(50080, 29), // 221 PtrCastToCrossWorkgroupINTEL
  IR(50139, 29), // 222 CrossWorkgroupCastToPtrINTEL
  IR(50191, 22), // 223 ReadPipeBlockingINTEL
  IR(50237, 23), // 224 WritePipeBlockingINTEL
  IR(51138, 26), // 225 ControlBarrierArriveINTEL
  IR(51186, 24), // 226 ControlBarrierWaitINTEL
  IR(51235, 24), // 227 TaskSequenceCreateINTEL
  IR(51283, 23), // 228 TaskSequenceAsyncINTEL
  IR(51328, 21), // 229 TaskSequenceGetINTEL
  IR(51375, 25), // 230 TaskSequenceReleaseINTEL
  IR(51423, 22), // 231 TypeTaskSequenceINTEL
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
  spv::Capability::SubgroupBufferPrefetchINTEL, // 330
  spv::Capability::Subgroup2DBlockTransformINTEL, // 331
  spv::Capability::Subgroup2DBlockTransposeINTEL, // 332
  spv::Capability::SubgroupMatrixMultiplyAccumulateINTEL, // 333
  spv::Capability::TernaryBitwiseFunctionINTEL, // 334
  spv::Capability::UntypedVariableLengthArrayINTEL, // 335
  spv::Capability::FunctionVariantsINTEL, // 336
  spv::Capability::PredicatedIOINTEL, // 337
  spv::Capability::GroupUniformArithmeticKHR, // 338
  spv::Capability::TensorFloat32RoundingINTEL, // 339
  spv::Capability::MaskedGatherScatterINTEL, // 340
  spv::Capability::BindlessImagesINTEL, // 341
  spv::Capability::DotProductFloat16AccFloat32VALVE, // 342
  spv::Capability::DotProductBFloat16AccVALVE, // 343
  spv::Capability::DotProductFloat16AccFloat16VALVE, // 344
  spv::Capability::DotProductBFloat16AccVALVE, // 345
  spv::Capability::DotProductFloat8AccFloat32VALVE, // 346
  spv::Capability::Float64, // 347
  spv::Capability::InterpolationFunction, // 348
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
  spvtools::Extension::kSPV_INTEL_subgroup_buffer_prefetch, // 187
  spvtools::Extension::kSPV_INTEL_2d_block_io, // 188
  spvtools::Extension::kSPV_INTEL_subgroup_matrix_multiply_accumulate, // 189
  spvtools::Extension::kSPV_INTEL_ternary_bitwise_function, // 190
  spvtools::Extension::kSPV_INTEL_function_variants, // 191
  spvtools::Extension::kSPV_INTEL_predicated_io, // 192
  spvtools::Extension::kSPV_INTEL_rounded_divide_sqrt, // 193
  spvtools::Extension::kSPV_KHR_uniform_group_instructions, // 194
  spvtools::Extension::kSPV_INTEL_tensor_float32_conversion, // 195
  spvtools::Extension::kSPV_INTEL_masked_gather_scatter, // 196
  spvtools::Extension::kSPV_INTEL_cache_controls, // 197
  spvtools::Extension::kSPV_INTEL_maximum_registers, // 198
  spvtools::Extension::kSPV_INTEL_bindless_images, // 199
  spvtools::Extension::kSPV_VALVE_mixed_float_dot_product, // 200
  spvtools::Extension::kSPV_SAMSUNG_intrinsic, // 201
  spvtools::Extension::kSPV_KHR_no_integer_wrap_decoration, // 202
  spvtools::Extension::kSPV_NV_mesh_shader, // 203
  spvtools::Extension::kSPV_EXT_mesh_shader, // 204
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 205
  spvtools::Extension::kSPV_GOOGLE_user_type, // 206
  spvtools::Extension::kSPV_KHR_subgroup_uniform_control_flow, // 207
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 208
  spvtools::Extension::kSPV_AMD_shader_early_and_late_fragment_tests, // 209
  spvtools::Extension::kSPV_EXT_shader_stencil_export, // 210
  spvtools::Extension::kSPV_KHR_maximal_reconvergence, // 211
  spvtools::Extension::kSPV_KHR_linkonce_odr, // 212
  spvtools::Extension::kSPV_KHR_storage_buffer_storage_class, // 213
  spvtools::Extension::kSPV_KHR_variable_pointers, // 214
  spvtools::Extension::kSPV_KHR_terminate_invocation, // 215
  spvtools::Extension::kSPV_KHR_relaxed_extended_instruction, // 216
  spvtools::Extension::kSPV_KHR_ray_tracing, // 217
  spvtools::Extension::kSPV_KHR_ray_query, // 218
  spvtools::Extension::kSPV_NV_ray_tracing, // 219
  spvtools::Extension::kSPV_KHR_ray_tracing, // 220
  spvtools::Extension::kSPV_KHR_ray_query, // 221
  spvtools::Extension::kSPV_NV_displacement_micromap, // 222
  spvtools::Extension::kSPV_GOOGLE_decorate_string, // 223
  spvtools::Extension::kSPV_GOOGLE_hlsl_functionality1, // 224
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
