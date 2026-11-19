// Test that a host which writes entry-point `uniform` parameters through
// reflection reads back the values that the shader sees when some of the
// parameters have `[vk::offset(N)]` (issue #9643).
//
// The test harness writes each `uniform` parameter at the offset that
// reflection reports, into a block of the size that reflection reports.
// On Vulkan `second` lives at byte 32, `third` follows it at byte 36, and
// `fourth` moves back to byte 8, so the block is 40 bytes long. On other
// targets `[vk::offset]` has no effect and the parameters keep their usual
// layout.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CHECK_WGSL): -target wgsl -entry computeMain -stage compute
//TEST:REFLECTION(filecheck=CHECK_REFL): -target spirv -entry computeMain -stage compute -no-codegen

// The WGSL emitter derives `@align` from the IR-layout offset, so `@align(4)`
// shows that `second` kept its natural offset 4 on WGSL; with the offset
// applied it would be `@align(16)`, for offset 32.
// CHECK_WGSL: struct EntryPointParams
// CHECK_WGSL-NEXT: {
// CHECK_WGSL-NEXT: first_0 : i32,
// CHECK_WGSL-NEXT: @align(4) second_0 : i32,

[numthreads(1, 1, 1)]
void computeMain(
//TEST_INPUT:uniform(data=[3]):name=first
    uniform int first,
//TEST_INPUT:uniform(data=[5]):name=second
    [vk::offset(32)] uniform int second,
//TEST_INPUT:uniform(data=[7]):name=third
    uniform int third,
//TEST_INPUT:uniform(data=[11]):name=fourth
    [vk::offset(8)] uniform int fourth,
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
    uniform RWStructuredBuffer<int> outputBuffer)
{
    // CHECK: type: int32_t
    // CHECK-NEXT: 3
    // CHECK-NEXT: 5
    // CHECK-NEXT: 7
    // CHECK-NEXT: 11
    outputBuffer[0] = first;
    outputBuffer[1] = second;
    outputBuffer[2] = third;
    outputBuffer[3] = fourth;
}

// Reflection places the parameters where the SPIR-V does, without a GPU.
// CHECK_REFL: "name": "first"
// CHECK_REFL: "binding": {"kind": "uniform", "offset": 0, "size": 4
// CHECK_REFL: "name": "second"
// CHECK_REFL: "binding": {"kind": "uniform", "offset": 32, "size": 4
// CHECK_REFL: "name": "third"
// CHECK_REFL: "binding": {"kind": "uniform", "offset": 36, "size": 4
// CHECK_REFL: "name": "fourth"
// CHECK_REFL: "binding": {"kind": "uniform", "offset": 8, "size": 4
