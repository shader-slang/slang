//TEST:SIMPLE(filecheck=METAL): -target metal -entry scalarMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry intMain -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry vector2Main -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry vector3Main -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry scalarMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry intMain -stage compute
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -entry vector2Main -stage compute

// On Metal, SV_GroupIndex is computed from the `uint3` [[thread_position_in_threadgroup]] value,
// reusing the entry point's own SV_GroupThreadID parameter. When that parameter is declared after
// SV_GroupIndex with a narrower type (`uint`, `int`, `uint2`), the group index must still be
// computed from the full `uint3`, not by subscripting the narrowed value (`gtid_1[int(2)]` on a
// `uint`, which Metal rejects).

RWStructuredBuffer<uint> dst;

// METAL: uint3 [[GTID:[A-Za-z0-9_]+]] {{\[\[}}thread_position_in_threadgroup
// METAL: [[GTID]]{{\[}}int(2)]
// METAL-SAME: [[GTID]]{{\[}}int(1)]
// METAL-SAME: [[GTID]]{{\[}}int(0)]
// METALLIB: @{{.*}}Main

[numthreads(2, 2, 1)]
void scalarMain(uint gi: SV_GroupIndex, uint gtid: SV_GroupThreadID)
{
    dst[gi] = gtid;
}

[numthreads(2, 2, 1)]
void intMain(uint gi: SV_GroupIndex, int gtid: SV_GroupThreadID)
{
    dst[gi] = uint(gtid);
}

[numthreads(2, 2, 1)]
void vector2Main(uint gi: SV_GroupIndex, uint2 gtid: SV_GroupThreadID)
{
    dst[gi] = gtid.x + gtid.y;
}

[numthreads(2, 2, 1)]
void vector3Main(uint3 gtid: SV_GroupThreadID, uint gi: SV_GroupIndex)
{
    dst[gi] = gtid.x + gtid.y;
}
