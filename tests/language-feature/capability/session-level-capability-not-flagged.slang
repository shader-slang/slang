// Regression test: capabilities set at the session level (via SessionDesc
// compilerOptionEntries) must NOT trigger E36121 even when the target does not
// support them.  This matches how multi-backend frameworks like SGL work: they
// add e.g. hlsl_nvapi at the session level (applicable to D3D12 targets) while
// the same session also contains a Vulkan/SPIRV target.
//
// The test simulates this at the slangc command-line level: a capability that
// would normally be incompatible with the target is accepted without error when
// passed at the global (session) level rather than the target level.
//
// We cannot express "session-level vs target-level" through slangc flags alone,
// so this test uses the existing -ignore-capabilities path to confirm the
// suppression mechanism is present, and a separate SIMPLE test confirms that
// the same capability on an incompatible target does produce E36121 from the
// target-level path.  The actual session-vs-target distinction is covered by
// the C++ API unit test in slang-unit-test-tool/.
//
// Verify E36121 fires when capability is incompatible with target (target-level).
//TEST:SIMPLE(filecheck=CHECK_ERR): -target hlsl -entry main -stage compute -capability spirv_1_5
// CHECK_ERR: error[E36121]

// Verify -ignore-capabilities suppresses the error.
//TEST:SIMPLE(filecheck=CHECK_IGNORE): -target hlsl -entry main -stage compute -capability spirv_1_5 -ignore-capabilities
// CHECK_IGNORE-NOT: E36121

[numthreads(1, 1, 1)]
void main() {}
