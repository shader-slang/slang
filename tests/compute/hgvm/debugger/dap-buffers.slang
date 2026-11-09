//TEST:DEBUGGER(mode=DAP, prefix=DAP):
//TEST:DEBUGGER(mode=DAP, prefix=DAP, precompile=2):

//DAP-SEND: {"command":"initialize"}
//DAP-SEND: {"command":"launch","arguments":{"program":"$program","api":"vulkan","gpuExecution":"buffered"}}
//DAP-SEND: wait initialized

struct Item
{
    Array<bool, 3> flags;
    float2 position;
    row_major float2x3 transform;
    uint id;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void fill(RWStructuredBuffer<Item> buffer, uint base, uint3 tid : SV_DispatchThreadID)
{
    Item item;
    item.flags[0] = true;
    item.flags[1] = tid.x % 2 == 0;
    item.flags[2] = false;
    item.position = float2(tid.x, base);
    item.transform = float2x3(float3(tid.x, 2, 3), float3(4, 5, base));
    item.id = base + tid.x;
    buffer[tid.x] = item;
}

[host]
void main()
{
    let buffer = RWStructuredBuffer<Item>::alloc(32);
    let alias = buffer;
    dispatch(fill, uint3(32, 1, 1))(buffer, 100u);
    //DAP-SEND: {"command":"setBreakpoints","arguments":{"source":{"path":"$file"},"breakpoints":[{"line":[[@LINE+1]]},{"line":[[@LINE+3]]}]}}
    print(1);
    dispatch(fill, uint3(32, 1, 1))(buffer, 200u);
    print(2);
    print(readBack(alias, 17).id);
}

//DAP-CHECK: "verified" : true
//DAP-SEND: {"command":"configurationDone"}
//DAP-SEND: wait stopped
//DAP-SEND: {"command":"stackTrace","arguments":{"threadId":1}}
//DAP-SEND: capture frame body.stackFrames.0.id
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"buffer"}}
//DAP-CHECK: "result" : "RWStructuredBuffer<Item> (count = 32,
//DAP-CHECK: "indexedVariables" : 32
//DAP-SEND: capture buffer body.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$buffer,"filter":"named"}}
//DAP-CHECK: "variables" : [
//DAP-CHECK-NEXT: ]
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$buffer,"start":17,"count":2}}
//DAP-CHECK: "name" : "[17]"
//DAP-CHECK: "name" : "[18]"
//DAP-SEND: capture item body.variables.0.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$item}}
//DAP-CHECK: "name" : "flags"
//DAP-CHECK: "name" : "position"
//DAP-CHECK: "value" : "(17, 100)"
//DAP-CHECK: "name" : "transform"
//DAP-CHECK: "name" : "id"
//DAP-CHECK: "value" : "117"
//DAP-SEND: capture flags body.variables.0.variablesReference
//DAP-SEND: capture matrix body.variables.2.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$flags}}
//DAP-CHECK: "value" : "true"
//DAP-CHECK: "value" : "false"
//DAP-CHECK: "value" : "false"
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$matrix}}
//DAP-CHECK: "value" : "(17, 2, 3)"
//DAP-CHECK: "value" : "(4, 5, 100)"
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"alias[18].position"}}
//DAP-CHECK: "result" : "(18, 100)"
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"buffer[32]"}}
//DAP-CHECK: "success" : false
//DAP-CHECK: Buffer index out of bounds.
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$buffer,"start":32,"count":1}}
//DAP-CHECK: "variables" : [
//DAP-CHECK-NEXT: ]
//DAP-SEND: {"command":"continue","arguments":{"threadId":1}}
//DAP-SEND: wait stopped
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$item}}
//DAP-CHECK: "success" : false
//DAP-CHECK: Invalid or expired variable reference.
//DAP-SEND: {"command":"stackTrace","arguments":{"threadId":1}}
//DAP-SEND: capture frame body.stackFrames.0.id
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"buffer[17].id"}}
//DAP-CHECK: "result" : "217"
//DAP-SEND: {"command":"continue","arguments":{"threadId":1}}
//DAP-SEND: wait terminated
//DAP-CHECK: "exitCode" : 0
//DAP-SEND: {"command":"disconnect"}
