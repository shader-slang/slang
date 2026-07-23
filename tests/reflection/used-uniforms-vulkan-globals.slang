// used-uniforms-vulkan-globals.slang

// Tests used byte range tracking for loose uniform globals, which on
// SPIR-V aggregate into an implicit $Globals buffer bound as a descriptor
// slot rather than the b register used-parameters.slang covers. Only
// usedScalar and two components of usedVec are read.

//TEST:REFLECTION:-stage compute -entry main -target spirv

uniform float usedScalar;
uniform float unusedScalar;
uniform float4 usedVec;

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = usedScalar + usedVec.x + usedVec.y;
}
