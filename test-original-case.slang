extern static const uint groupSizeX;
extern static const uint groupSizeY;
extern static const uint groupSizeZ;

[shader("compute")]
[numthreads(groupSizeX, groupSizeY, groupSizeZ)]
void kernel1(uint3 tid : SV_DispatchThreadID) {}