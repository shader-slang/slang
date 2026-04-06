//TEST:SIMPLE(filecheck=SPIRV): -target spirv
// SPIR-V-only: checks OpSpecConstant/SpecId emission for enum specialization constants,
// including a plain enum class that uses the default int backing type.

// SPIRV-DAG: OpDecorate %[[MODE_B:[0-9A-Za-z_]+]] SpecId 11
// SPIRV-DAG: %[[MODE_A:[0-9A-Za-z_]+]] = OpSpecConstant %int 2
// SPIRV-DAG: %[[MODE_B]] = OpSpecConstant %int 9

enum class Mode
{
    Add = 2,
    Multiply = 9,
}

[SpecializationConstant]
const Mode modeA = Mode::Add;

[vk::constant_id(11)]
const Mode modeB = Mode::Multiply;

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[id.x] = (int)modeA + (int)modeB;
}
