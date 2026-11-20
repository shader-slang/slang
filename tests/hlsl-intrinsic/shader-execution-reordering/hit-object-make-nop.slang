// hit-object-make-nop.slang

//TEST:SIMPLE: -target dxil -entry rayGenerationMain -stage raygeneration -profile sm_6_5 -DNV_SHADER_EXTN_SLOT=u0
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry rayGenerationMain -stage raygeneration -profile sm_6_5 -line-directive-mode none -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage raygeneration -entry rayGenerationMain -line-directive-mode none


//TEST_INPUT:set outputBuffer = out ubuffer(data=[0, 0, 0, 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

void rayGenerationMain()
{
    int2 launchID = int2(DispatchRaysIndex().xy);
    int2 launchSize = int2(DispatchRaysDimensions().xy);

    int idx = launchID.x;

    // SPIRV: OpHitObjectRecordEmptyNV
    // GLSL: hitObjectRecordEmptyEXT
    HitObject hit = HitObject::MakeNop();

    // GLSL: hitObjectIsEmptyEXT
    outputBuffer[idx] = uint(hit.IsNop());
}
