//TEST:DEBUGGER(mode=DAP, prefix=DAP):

// Pause and breakpoint edits must work while the program is running without single-stepping.
//DAP-SEND: {"command":"initialize"}
//DAP-SEND: {"command":"launch","arguments":{"program":"$program","stopOnEntry":true}}
//DAP-SEND: wait initialized
//DAP-SEND: {"command":"configurationDone"}
//DAP-SEND: wait stopped
//DAP-CHECK: "reason" : "entry"
//DAP-SEND: {"command":"continue","arguments":{"threadId":1}}
//DAP-SEND: {"command":"pause","arguments":{"threadId":1}}
//DAP-SEND: wait stopped
//DAP-CHECK: "reason" : "pause"
//DAP-SEND: {"command":"continue","arguments":{"threadId":1}}

[host]
void main()
{
    uint i = 0;
    for (uint iteration = 0; iteration < 0xfffffffe; ++iteration)
    {
        //DAP-SEND: {"command":"setBreakpoints","arguments":{"source":{"path":"$file"},"breakpoints":[{"line":[[@LINE+1]]}]}}
        i += 1;
    }
    print(i);
}

//DAP-CHECK: "id" : [[BREAKPOINT:[0-9]+]]
//DAP-CHECK: "verified" : true
//DAP-SEND: capture breakpoint body.breakpoints.0.id
//DAP-SEND: capture line body.breakpoints.0.line
//DAP-SEND: wait stopped
//DAP-CHECK: "reason" : "breakpoint"
//DAP-SEND: {"command":"setBreakpoints","arguments":{"source":{"path":"$file"},"breakpoints":[{"line":$line}]}}
//DAP-CHECK: "id" : [[BREAKPOINT]]
//DAP-CHECK: "verified" : true
//DAP-SEND: {"command":"setBreakpoints","arguments":{"source":{"path":"$file"},"breakpoints":[]}}
//DAP-SEND: {"command":"setFunctionBreakpoints","arguments":{"breakpoints":[{"name":"notAFunction"}]}}
//DAP-CHECK: "verified" : false
//DAP-SEND: {"command":"next","arguments":{"threadId":1}}
//DAP-SEND: wait stopped
//DAP-CHECK: "reason" : "step"
//DAP-SEND: {"command":"continue","arguments":{"threadId":1}}
//DAP-SEND: {"command":"terminate"}
//DAP-SEND: wait terminated
//DAP-CHECK: "command" : "terminate"
//DAP-CHECK: "success" : true
//DAP-CHECK: "event" : "exited"
//DAP-CHECK: "exitCode" : 0
//DAP-CHECK: "event" : "terminated"
//DAP-SEND: {"command":"disconnect"}
