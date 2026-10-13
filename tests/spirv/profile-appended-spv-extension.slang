// A SPIR-V extension capability may be spelled either `SPV_*` or `spv<Feature>`. When such an atom
// is appended to a `-profile`, `TargetRequest::getTargetCaps()` must pull it into the target caps
// (via `isSpirvExtensionAtom`), so an entry point that uses it is already covered by the profile.
//
// `spvImageGatherExtended` exists only in the `spv<Feature>` spelling, so this exercises the
// `spv[A-Z]` branch of `isSpirvExtensionAtom` specifically. `GatherRed` with a constant offset
// requires it.

// Control: without the extension in the profile, the profile is implicitly upgraded (E41012)
// to include the `spvImageGatherExtended` capability that `GatherRed` with an offset needs.
//TEST:SIMPLE(filecheck=WARN): -target spirv-asm -emit-spirv-directly -profile spirv_1_5 -entry main -stage compute
//WARN: E41012
//WARN: spvImageGatherExtended

// With the extension appended, it is pulled into the target caps up front, so no upgrade warning
// is emitted and the module compiles at 1.5.
//TEST:SIMPLE(filecheck=OK): -target spirv-asm -emit-spirv-directly -profile spirv_1_5+spvImageGatherExtended -entry main -stage compute
//OK-NOT: E41012
//OK: ; Version: 1.5

Texture2D tex;
SamplerState s;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = tex.GatherRed(s, float2(0, 0), int2(1, 1));
}
