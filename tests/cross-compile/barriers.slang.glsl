#version 450

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    (memoryBarrier(), groupMemoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer());
    (memoryBarrier(), groupMemoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer(), barrier());
    (memoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer());
    (memoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer(), barrier());
    groupMemoryBarrier();
    (groupMemoryBarrier(), barrier());

    return;
}
