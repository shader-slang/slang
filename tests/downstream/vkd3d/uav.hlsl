//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc-asm -profile cs_5_0 -entry computeMain -pass-through vkd3d
//CHECK: cs_5_0
//CHECK: dcl_uav_typed
//CHECK: dcl_uav_structured
//CHECK: dcl_uav_raw
//CHECK: store_uav_typed
//CHECK: store_structured
//CHECK: store_raw

RWBuffer<uint> typedValues : register(u0);
RWStructuredBuffer<uint> structuredValues : register(u1);
RWByteAddressBuffer rawValues : register(u2);

[numthreads(4, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    typedValues[id.x] = id.x + 1;
    structuredValues[id.x] = id.x + 2;
    rawValues.Store(id.x * 4, id.x + 3);
}
