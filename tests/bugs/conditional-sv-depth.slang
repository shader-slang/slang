// conditional-sv-depth.slang
// Test that Conditional<float, true> works correctly with SV_Depth semantic.
// https://github.com/shader-slang/slang/issues/XXXX
// 
// When using Conditional<T, true> which internally becomes T[1],
// the SPIR-V emitter should correctly unwrap the single-element array
// for FragDepth which must be a scalar float in Vulkan.
//
// This test verifies that the SPIR-V compilation succeeds and produces
// valid output that would pass spirv-val with --target-env vulkan1.2

//TEST:COMPILE: tests/bugs/conditional-sv-depth.slang -target spirv -emit-spirv-directly -o tests/bugs/conditional-sv-depth.spv

[shader("fragment")]
Conditional<float, true> ps_main(): SV_Depth {
    return 0.0;
}
