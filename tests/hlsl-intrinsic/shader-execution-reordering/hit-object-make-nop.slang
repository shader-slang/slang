// hit-object-make-nop.slang

//TEST:SIMPLE: -target dxil -entry rayGenerationMain -stage raygeneration -profile sm_6_5 -DNV_SHADER_EXTN_SLOT=u0 
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry rayGenerationMain -stage raygeneration -profile glsl_460+GL_EXT_ray_tracing -O0 -line-directive-mode none 
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry rayGenerationMain -stage raygeneration -O0  -emit-spirv-directly

//DISABLE_TEST(compute):COMPARE_COMPUTE:-d3d12 -output-using-type -use-dxil -profile sm_6_5 -render-feature ray-query
//DISABLE_TEST(compute):COMPARE_COMPUTE:-vk -output-using-type -render-feature ray-query

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0, 0, 0, 0], stride=4)

// SPIRV: OpHitObjectRecordEmptyNV
// GLSL: hitObjectRecordEmptyNV

RWStructuredBuffer<uint> outputBuffer;

void rayGenerationMain() 
{
    int2 launchID = int2(DispatchRaysIndex().xy);
    int2 launchSize = int2(DispatchRaysDimensions().xy);

    int idx = launchID.x;

    HitObject hit = HitObject::MakeNop();
    
    outputBuffer[idx] = uint(hit.IsNop());
}
