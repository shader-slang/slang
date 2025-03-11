// module-dual-target-verify.slang
// Test that a slang-module can retrieve both SPIR-V and DXIL targets independently
// and one does not affect the other

//TEST:SIMPLE(filecheck=CHECK):-multi-module -verify-module-dual-target

// First create a module that has both DXIL and SPIRV embedded
// CHECK: Creating module with both DXIL and SPIRV embedded

// Verify both targets are present
// CHECK: Precompiled blob for target DXIL exists: true
// CHECK: Precompiled blob for target SPIRV exists: true

// Retrieve and check DXIL content
// CHECK: Retrieved DXIL blob size: {{[1-9][0-9]+}}

// Retrieve and check SPIRV content
// CHECK: Retrieved SPIRV blob size: {{[1-9][0-9]+}}

// When retrieving DXIL, SPIRV should remain untouched
// CHECK: DXIL retrieval does not modify SPIRV blob

// When retrieving SPIRV, DXIL should remain untouched
// CHECK: SPIRV retrieval does not modify DXIL blob

// Both blobs should have distinct hash values
// CHECK-NOT: DXIL hash matches SPIRV hash
// CHECK: DXIL and SPIRV blobs are separate 