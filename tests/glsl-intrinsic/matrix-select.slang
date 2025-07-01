//TEST:SIMPLE(filecheck=CHK): -target spirv -entry computeMain -stage compute

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    //CHK: error 36107: entrypoint 'computeMain' uses features
    float2x3 a2x3 = 1 << 6;
    float2x3 b2x3 = 0;
    float2x3 c2x3 = select(bool2x3(true), a2x3, b2x3);
}
