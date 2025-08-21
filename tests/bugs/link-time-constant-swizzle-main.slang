//TEST:SIMPLE: -stage compute -entry computeMain

extern static const uint3 groupSize = uint3(8, 8, 1);

[shader("compute")]
[numthreads(groupSize.x, groupSize.y, groupSize.z)]
void computeMain(uint3 tid : SV_DispatchThreadID) 
{
}