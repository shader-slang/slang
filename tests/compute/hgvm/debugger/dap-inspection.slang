//TEST:DEBUGGER(mode=DAP, prefix=DAP):
//TEST:DEBUGGER(mode=DAP, prefix=DAP, precompile=2):

//DAP-SEND: {"command":"initialize","arguments":{"linesStartAt1":true,"columnsStartAt1":true}}
//DAP-CHECK: "supportsConfigurationDoneRequest" : true
//DAP-SEND: {"command":"launch","arguments":{"program":"$program"}}
//DAP-SEND: wait initialized

struct Leaf { int value; }
struct Branch { Leaf leaf; }
struct Tree { Branch branch; }
struct Root { Tree tree; }

[ForceInline]
int inspect(inout int value)
{
    int items[20];
    for (int i = 0; i < 20; ++i)
        items[i] = i;
    Root deep;
    deep.tree.branch.leaf.value = 42;
    float2 position = float2(100, 200);
    float2x2 matrix = float2x2(1, 2, 3, 4);
    bool3 flags = bool3(true, false, true);
    value += 3;
    //DAP-SEND: {"command":"setBreakpoints","arguments":{"source":{"path":"$file"},"breakpoints":[{"line":[[@LINE+1]]}]}}
    print(value);
    return items[19] + deep.tree.branch.leaf.value;
}

[host]
void main()
{
    int value = 4;
    int result = inspect(value);
    print(result);
}

//DAP-CHECK: "verified" : true
//DAP-SEND: {"command":"configurationDone"}
//DAP-SEND: wait stopped
//DAP-CHECK: "reason" : "breakpoint"
//DAP-SEND: {"command":"stackTrace","arguments":{"threadId":1}}
//DAP-CHECK: "id" : [[FRAME:[0-9]+]]
//DAP-CHECK: "name" : "inspect (inlined)"
//DAP-CHECK: "name" : "main"
//DAP-SEND: capture frame body.stackFrames.0.id
//DAP-SEND: {"command":"stackTrace","arguments":{"threadId":1,"startFrame":0,"levels":1}}
//DAP-CHECK: "id" : [[FRAME]]
//DAP-CHECK: "name" : "inspect (inlined)"
//DAP-SEND: {"command":"scopes","arguments":{"frameId":$frame}}
//DAP-CHECK: "name" : "Arguments"
//DAP-CHECK: "name" : "Locals"
//DAP-SEND: capture args body.scopes.0.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$args}}
//DAP-CHECK: "name" : "value"
//DAP-CHECK: "value" : "7"
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"position"}}
//DAP-CHECK: "result" : "(100, 200)"
//DAP-CHECK: "variablesReference" : 0
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"matrix[1]"}}
//DAP-CHECK: "result" : "(3, 4)"
//DAP-CHECK: "variablesReference" : 0
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"flags"}}
//DAP-CHECK: "result" : "(true, false, true)"
//DAP-CHECK: "variablesReference" : 0
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"items"}}
//DAP-CHECK: "indexedVariables" : 20
//DAP-SEND: capture array body.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$array,"start":17,"count":3}}
//DAP-CHECK: "name" : "[17]"
//DAP-CHECK: "value" : "17"
//DAP-CHECK: "name" : "[19]"
//DAP-CHECK: "value" : "19"
//DAP-SEND: {"command":"evaluate","arguments":{"frameId":$frame,"expression":"deep"}}
//DAP-CHECK: "result" : "Root"
//DAP-CHECK: "type" : "Root"
//DAP-SEND: capture root body.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$root}}
//DAP-CHECK: "name" : "tree"
//DAP-SEND: capture tree body.variables.0.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$tree}}
//DAP-CHECK: "name" : "branch"
//DAP-SEND: capture branch body.variables.0.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$branch}}
//DAP-CHECK: "name" : "leaf"
//DAP-SEND: capture leaf body.variables.0.variablesReference
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$leaf}}
//DAP-CHECK: "name" : "value"
//DAP-CHECK: "value" : "42"
//DAP-SEND: {"command":"setBreakpoints","arguments":{"source":{"path":"$file"},"breakpoints":[]}}
//DAP-SEND: {"command":"stepOut","arguments":{"threadId":1}}
//DAP-SEND: wait stopped
//DAP-CHECK: "output" : "7\n"
//DAP-CHECK: "reason" : "step"
//DAP-SEND: {"command":"variables","arguments":{"variablesReference":$array}}
//DAP-CHECK: "success" : false
//DAP-CHECK: "message" : "Invalid or expired variable reference."
//DAP-SEND: {"command":"stackTrace","arguments":{"threadId":1}}
//DAP-CHECK: "name" : "main"
//DAP-CHECK: "totalFrames" : 1
//DAP-SEND: {"command":"continue","arguments":{"threadId":1}}
//DAP-SEND: wait terminated
//DAP-CHECK: "output" : "61\n"
//DAP-CHECK: "exitCode" : 0
//DAP-SEND: {"command":"disconnect"}
