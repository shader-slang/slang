//TEST:DEBUGGER(mode=CLI, prefix=CLI): -api vulkan
//TEST:DEBUGGER(mode=CLI, prefix=CLI): -api vulkan -gpu-execution buffered
//TEST:DEBUGGER(mode=CLI, prefix=CLI, precompile=2): -api vulkan -gpu-execution synchronous

[shader("compute")]
[numthreads(1, 1, 1)]
void fill(RWStructuredBuffer<float2> buffer, uint3 tid : SV_DispatchThreadID)
{
    buffer[tid.x] = float2(tid.x, tid.x + 100);
}

RWStructuredBuffer<uint> restrictedBuffer()
{
    uint one = 1;
    return hgbc_asm {
        result : $$RWStructuredBuffer<uint> = OpAllocateResource
            Buffer Buffer Undefined DeviceLocal UnorderedAccess $$uint
            $one $one $one 1 1 0 ShaderResource UnorderedAccess;
    };
}

[host]
void main()
{
    let buffer = RWStructuredBuffer<float2>::alloc(20);
    dispatch(fill, uint3(20, 1, 1))(buffer);
    let readOnly = StructuredBuffer<float2>::alloc(20);
    copy(readOnly, 0, buffer, 0, 20);
    let restricted = restrictedBuffer();
    //CLI-SEND: break '$file':[[@LINE+1]]
    print(readBack(buffer, 19).x);
}

//CLI-SEND: run
//CLI-SEND: info locals
//CLI-CHECK: buffer = RWStructuredBuffer<vector<float,2>> (count = 20,
//CLI-SEND: whatis buffer[19]
//CLI-CHECK: type = vector<float,2>
//CLI-SEND: print buffer[19]
//CLI-CHECK: $1 = (19, 119)
//CLI-SEND: print buffer[19].y
//CLI-CHECK: $2 = 119
//CLI-SEND: print buffer
//CLI-CHECK: $3 = RWStructuredBuffer<vector<float,2>>
//CLI-CHECK-SAME: [0] = (0, 100)
//CLI-CHECK-SAME: [15] = (15, 115)
//CLI-CHECK-SAME: ...
//CLI-SEND: print buffer[20]
//CLI-CHECK: Buffer index out of bounds.
//CLI-SEND: print readOnly[19]
//CLI-CHECK: $4 = (19, 119)
//CLI-SEND: whatis restricted[0]
//CLI-CHECK: type = uint
//CLI-SEND: print restricted[0]
//CLI-CHECK: Buffer inspection requires CopySource usage.
//CLI-SEND: continue
//CLI-CHECK: Program exited normally.
//CLI-SEND: quit
//CLI-CHECK: standard output = {
//CLI-CHECK-NEXT: 19
//CLI-CHECK-NEXT: }
