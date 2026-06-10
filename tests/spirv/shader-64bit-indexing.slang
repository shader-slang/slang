//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -emit-spirv-directly

// Regression test for the spvShader64BitIndexingEXT capability atom (issue #11538).
//
// The [require(spvShader64BitIndexingEXT)] clause below only type-checks once the atom exists
// in slang-capabilities.capdef; before that it is an unknown-capability error. The inline
// spirv_asm block declares the matching SPIR-V extension and capability so we can FileCheck
// that both are emitted. OpCapability precedes OpExtension in SPIR-V logical layout.

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[require(spvShader64BitIndexingEXT)]
int use64BitIndexing(int n)
{
    return spirv_asm
    {
        OpExtension "SPV_EXT_shader_64bit_indexing";
        OpCapability Shader64BitIndexingEXT;
        OpConstant $$int %two 2;
        OpIMul $$int result $n %two
    };
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    outputBuffer[i] = use64BitIndexing(outputBuffer[i]);
}

// CHECK: OpCapability Shader64BitIndexingEXT
// CHECK: OpExtension "SPV_EXT_shader_64bit_indexing"
