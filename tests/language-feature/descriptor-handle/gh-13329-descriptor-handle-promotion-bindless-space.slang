// A target with no profile or capability reserves a bindless heap space for `DescriptorHandle`
// only if the `descriptor_handle` promotion leaves its caps implying the capability. Plain `hlsl`
// does not imply `_sm_6_6`, so the promotion is what reserves the space on HLSL. The LLVM CPU path
// is outside the `descriptor_handle` alias, so the promotion leaves it alone and no space is
// reserved, the same as an explicit `-capability descriptor_handle`. See shader-slang/slang#13329.

//TEST:REFLECTION(filecheck=HLSL):-target hlsl -stage compute -entry computeMain -no-codegen
//TEST:REFLECTION(filecheck=LLVM):-target llvm-ir -stage compute -entry computeMain -no-codegen

// HLSL: "bindlessSpaceIndex": {{[0-9]+}}

// LLVM-NOT: "bindlessSpaceIndex"
// LLVM: "threadGroupSize"
// LLVM-NOT: "bindlessSpaceIndex"

uniform DescriptorHandle<RWStructuredBuffer<uint>> gCounter;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    gCounter[0] = 1;
}
