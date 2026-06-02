//TEST:SIMPLE(filecheck=SPIRV): -target spirv
// SPIR-V-only: checks OpSpecConstant/SpecId emission for enum typedef specialization constants.

// SPIRV-DAG: OpDecorate %[[MODE:[0-9A-Za-z_]+]] SpecId 5
// SPIRV-DAG: %[[MODE]] = OpSpecConstant %uint 2

enum class Mode : uint32_t
{
    Add = 2,
    Multiply = 9,
}

typedef Mode ModeTypedef;

[vk::constant_id(5)]
const ModeTypedef mode = Mode::Add;

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[id.x] = (uint)mode;
}
