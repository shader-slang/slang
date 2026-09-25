// The optional coherence hint and bit count follow the payload. They must remain ordinary
// operands: the empty-payload replacement belongs to the opcode's defined payload position.

//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderEXT -O0 -validate-ir
//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage raygeneration -target spirv -capability spvShaderInvocationReorderEXT -O3 -validate-ir

struct Empty
{
}

[shader("raygeneration")]
void main()
{
    HitObject hit = HitObject::MakeNop();
    Empty payload;
    uint hint = 3;
    uint bitCount = 2;
    spirv_asm
    {
        OpExtension "SPV_EXT_shader_invocation_reorder";
        OpCapability ShaderInvocationReorderEXT;
        OpHitObjectReorderExecuteShaderEXT &hit &payload $hint $bitCount;
    };
}

// SPIRV-DAG: %[[hint:[A-Za-z0-9_]+]] = OpConstant %{{[A-Za-z0-9_]+}} 3
// SPIRV-DAG: %[[bits:[A-Za-z0-9_]+]] = OpConstant %{{[A-Za-z0-9_]+}} 2
// SPIRV-DAG: %[[uint:[A-Za-z0-9_]+]] = OpTypeInt 32 0
// SPIRV-DAG: %[[empty:[A-Za-z0-9_]+]] = OpTypeStruct %[[uint]]{{$}}
// SPIRV-DAG: %[[pointer:[A-Za-z0-9_]+]] = OpTypePointer RayPayloadKHR %[[empty]]
// SPIRV-DAG: %[[payload:[A-Za-z0-9_]+]] = OpVariable %[[pointer]] RayPayloadKHR
// SPIRV: OpHitObjectReorderExecuteShaderEXT %{{[A-Za-z0-9_]+}} %[[payload]] %[[hint]] %[[bits]]{{$}}
