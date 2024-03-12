//TEST:SIMPLE(filecheck=SPIRV): -stage compute -entry computeMain -target spirv -emit-spirv-directly -fspv-reflect

// Test ability to directly output SPIR-V reflection info on parameters.

Texture2D mTexture : MY_HLSL_SEMANTIC;
struct MyElement
{
    float x;
}
StructuredBuffer<MyElement> myBuffer : MY_HLSL_SEMANTIC1;
AppendStructuredBuffer<float> myAppendBuffer : MY_HLSL_SEMANTIC2;
ByteAddressBuffer bw;
[numthreads(8, 8, 1)]
void computeMain()
{
    float4 color = mTexture.Load(int3(0, 0, 0));
    myAppendBuffer.Append(color.x + myBuffer[0].x + bw.Load(0));
}

// SPIRV-DAG: OpExtension "SPV_GOOGLE_user_type"

// SPIRV-DAG: OpDecorate %mTexture UserTypeGOOGLE "texture2d"
// SPIRV-DAG: OpDecorate %mTexture UserSemantic "MY_HLSL_SEMANTIC"

// SPIRV-DAG: OpDecorate %myBuffer UserTypeGOOGLE "structuredbuffer:<MyElement>"
// SPIRV-DAG: OpDecorate %myBuffer UserSemantic "MY_HLSL_SEMANTIC1"

// SPIRV-DAG: OpDecorate %myAppendBuffer_counter UserTypeGOOGLE "appendstructuredbuffer:<float>"
// SPIRV-DAG: OpDecorate %myAppendBuffer_counter UserSemantic "MY_HLSL_SEMANTIC2"

// SPIRV-DAG: OpDecorate %myAppendBuffer_elements UserTypeGOOGLE "appendstructuredbuffer:<float>"
// SPIRV-DAG: OpDecorate %myAppendBuffer_elements UserSemantic "MY_HLSL_SEMANTIC2"
// SPIRV-DAG: OpDecorateId %myAppendBuffer_elements CounterBuffer %myAppendBuffer_counter

// SPIRV-DAG: OpDecorate %bw UserTypeGOOGLE "byteaddressbuffer"