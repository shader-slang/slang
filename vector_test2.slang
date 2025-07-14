[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint3 result = uint3(1, 2, 3);
    uint x = result.x;
    uint y = result.y; 
    uint z = result.z;
}
