//TEST:SIMPLE(filecheck=SPIRV): -target spirv
// SPIR-V-only: checks OpSpecConstant/SpecId emission for signed/unsigned enum specialization constants.

// SPIRV-DAG: OpDecorate %[[SIGNED:[0-9A-Za-z_]+]] SpecId 21
// SPIRV-DAG: OpDecorate %[[UNSIGNED:[0-9A-Za-z_]+]] SpecId 22
// SPIRV-DAG: %[[SIGNED]] = OpSpecConstant %int -3
// SPIRV-DAG: %[[UNSIGNED]] = OpSpecConstant %uint 7

enum class SignedMode : int
{
    Neg = -3,
}

enum class UnsignedMode : uint
{
    Seven = 7,
}

[vk::constant_id(21)]
const SignedMode signedMode = SignedMode::Neg;

[vk::constant_id(22)]
const UnsignedMode unsignedMode = UnsignedMode::Seven;

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[0] = (int)signedMode;
    outputBuffer[1] = (int)(uint)unsignedMode;
}
