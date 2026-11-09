//TEST:DEBUGGER(mode=DAP, prefix=DAP):

// Removing a user breakpoint while next runs must preserve the shared temporary breakpoint bit.
//DAP-SEND: {"command":"initialize"}
//DAP-SEND: {"command":"launch","arguments":{"program":"$program","stopOnEntry":true}}
//DAP-SEND: wait initialized

[noinline]
int work()
{
    int value = 0;
    for (int i = 0; i < 3000000; ++i)
        value += 1;
    return value;
}

[host]
void main()
{
    int value = work();
    //DAP-SEND: {"command":"setBreakpoints","arguments":{"source":{"path":"$file"},"breakpoints":[{"line":[[@LINE+1]]}]}}
    print(value);
}

//DAP-CHECK: "verified" : true
//DAP-SEND: {"command":"configurationDone"}
//DAP-SEND: wait stopped
//DAP-CHECK: "reason" : "entry"
//DAP-SEND: {"command":"next","arguments":{"threadId":1}}
//DAP-SEND: {"command":"setBreakpoints","arguments":{"source":{"path":"$file"},"breakpoints":[]}}
//DAP-CHECK: "command" : "setBreakpoints"
//DAP-CHECK: "success" : true
//DAP-SEND: wait stopped
//DAP-CHECK: "reason" : "step"
//DAP-SEND: {"command":"stackTrace","arguments":{"threadId":1}}
//DAP-CHECK: "name" : "main"
//DAP-CHECK: "totalFrames" : 1
//DAP-SEND: {"command":"evaluate","arguments":{"expression":"value"}}
//DAP-CHECK: "result" : "3000000"
//DAP-SEND: {"command":"continue","arguments":{"threadId":1}}
//DAP-SEND: wait terminated
//DAP-CHECK: "output" : "3000000\n"
//DAP-CHECK: "exitCode" : 0
//DAP-SEND: {"command":"disconnect"}
