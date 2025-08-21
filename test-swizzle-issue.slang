extern static const uint groupSizeX;
extern static const uint groupSizeY;
extern static const uint groupSizeZ;

[shader("compute")]
[numthreads(groupSizeX, groupSizeY, groupSizeZ)]
void kernel1(uint3 tid : SV_DispatchThreadID) {}

extern static const uint3 groupSize;

// Error here
[shader("compute")]
[numthreads(groupSize.x, groupSize.y, groupSize.z)]
void kernel2(uint3 tid : SV_DispatchThreadID) {}