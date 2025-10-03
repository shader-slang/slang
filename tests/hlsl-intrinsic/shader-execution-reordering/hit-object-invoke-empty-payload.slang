// hit-object-invoke-empty-payload.slang

// Test that HitObject.Invoke works with empty payload structs
// Regression test for issue where empty payload structs caused internal error during type legalization

//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry rayGenerationMain -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_5

[raypayload]
struct EmptyPayload
{
};

uniform RaytracingAccelerationStructure sceneBVH;
uniform RWStructuredBuffer<uint> outputBuffer;

void rayGenerationMain() 
{
    int2 launchID = int2(DispatchRaysIndex().xy);
    int idx = launchID.x;

    // SPIRV: OpHitObjectRecordEmptyNV
    HitObject hitObjectNOP = HitObject::MakeNop();
    
    EmptyPayload payload = {};
    // HLSL: NvInvokeHitObject(
    HitObject::Invoke(sceneBVH, hitObjectNOP, payload);
    
    // SPIRV: OpHitObjectIsEmptyNV
    outputBuffer[idx] = uint(hitObjectNOP.IsNop());
}
