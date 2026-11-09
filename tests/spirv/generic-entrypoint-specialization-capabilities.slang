// Specialization requirements must reach code generation even when the argument is unused.
//TEST:SIMPLE(filecheck=INDEXING): -target spirv-asm -entry genericMain<NeedsIndexing>
//TEST:SIMPLE(filecheck=INDEXING): -target spirv-asm -entry genericMain -specialize NeedsIndexing
//TEST:SIMPLE(filecheck=VERSION): -target spirv-asm -entry genericMain<NeedsSpirv16> -profile spirv_1_5
//TEST:SIMPLE(filecheck=VERSION): -target spirv-asm -entry genericMain -specialize NeedsSpirv16 -profile spirv_1_5
//TEST:SIMPLE(filecheck=PLAIN): -target spirv-asm -entry genericMain<Plain> -profile spirv_1_5
//TEST:SIMPLE(filecheck=PLAIN): -target spirv-asm -entry genericMain -specialize Plain -profile spirv_1_5
//DIAGNOSTIC_TEST:SIMPLE(diag=RESTRICTIVE,non-exhaustive): -target spirv-asm -entry genericMain<NeedsSpirv16> -profile spirv_1_5 -restrictive-capability-check
//DIAGNOSTIC_TEST:SIMPLE(diag=RESTRICTIVE,non-exhaustive): -target spirv-asm -entry genericMain -specialize NeedsSpirv16 -profile spirv_1_5 -restrictive-capability-check

//INDEXING-DAG: OpCapability Shader64BitIndexingEXT
//INDEXING-DAG: OpExtension "SPV_EXT_shader_64bit_indexing"
//INDEXING-DAG: OpExecutionMode %genericMain Shader64BitIndexingEXT
//VERSION: Version: 1.6
//PLAIN: Version: 1.5
//PLAIN-NOT: Shader64BitIndexingEXT
//PLAIN: OpFunctionEnd
//RESTRICTIVE: E41013

[require(spvShader64BitIndexingEXT)]
struct NeedsIndexing
{
    uint value;
}

// Use the version atom because the public spirv_1_6 alias also allows earlier versions.
[require(_spirv_1_6)]
struct NeedsSpirv16
{
    uint value;
}

struct Plain
{
    uint value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void genericMain<T>()
{
}
