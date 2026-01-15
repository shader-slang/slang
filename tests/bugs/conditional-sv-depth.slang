// conditional-sv-depth.slang
// Test that Conditional<float, true> works correctly with SV_Depth semantic.
// https://github.com/shader-slang/slang/issues/XXXX
// 
// When using Conditional<T, true> which internally becomes T[1],
// the SPIR-V emitter should correctly unwrap the single-element array
// for FragDepth which must be a scalar float in Vulkan.
//
// This test verifies that:
// 1. gl_FragDepth is declared as a scalar float pointer, not an array
// 2. The generated SPIR-V passes spirv-val validation

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry ps_main -emit-spirv-directly

// CHECK: %_ptr_Output_float = OpTypePointer Output %float
// CHECK: %gl_FragDepth = OpVariable %_ptr_Output_float Output
// CHECK-NOT: %_ptr_Output__arr_float
// CHECK: OpStore %gl_FragDepth %

uniform float depthValue;

[shader("fragment")]
Conditional<float, true> ps_main(): SV_Depth {
    return depthValue;
}
