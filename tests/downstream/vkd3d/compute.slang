//TEST:SIMPLE(filecheck=CHECK):-O1 -target dxbc-asm -profile cs_5_0 -entry computeMain
//CHECK: cs_5_0
//CHECK: dcl_thread_group 4, 1, 1
//CHECK: store_structured
//CHECK: ret

RWStructuredBuffer<uint> values;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    values[id.x] = id.x + 10;
}
