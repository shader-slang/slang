//TEST:COMPILE: tests/bugs/13306/static-array.slang -target dxil -profile cs_6_0 -entry main -validate-ir

// `MoveGlobalVarInitializationToEntryPointsPass` must expose array-valued initialization to
// `legalizeArrayReturnType` before HLSL emission. This also applies to ordinary `static` arrays.
uniform uint input[2];
static uint copy[2] = input;
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    copy[1] += tid.x;
    output[tid.x] = copy[1];
}
