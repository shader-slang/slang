//TEST:SIMPLE(filecheck=SPIRV): -target spirv
// SPIR-V-only: checks OpSpecConstant/SpecId emission for enum specialization constants.

// SPIRV-DAG: OpDecorate %[[MODE_B:[0-9A-Za-z_]+]] SpecId 11
// SPIRV-DAG: %[[MODE_A:[0-9A-Za-z_]+]] = OpSpecConstant %uint 2
// SPIRV-DAG: %[[MODE_B]] = OpSpecConstant %uint 9

enum class Mode : uint32_t
{
    Add = 2,
    Multiply = 9,
}

[SpecializationConstant]
const Mode modeA = Mode::Add;

[vk::constant_id(11)]
const Mode modeB = Mode::Multiply;

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[id.x] = (uint)modeA + (uint)modeB;
}
