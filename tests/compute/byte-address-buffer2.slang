// byte-address-buffer.slang

//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//DISABLED_TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX:-d3d12 -compute -shaderobj
//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute
//DISABLED_TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//DISABLED_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute

// Confirm cross-compilation of `(RW)ByteAddressBuffer`
//
// TODO: I'm only using `RWByteAddressBuffer` for now because I don't
// know if `render-test` supports the non-UAV case.

[vk::binding(2, 3)] RWByteAddressBuffer buffer0;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK-NOT: warning
	buffer0.Store(10, buffer0.Load<float4>(32));
}