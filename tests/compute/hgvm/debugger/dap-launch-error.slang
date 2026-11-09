//TEST:DEBUGGER(mode=DAP, prefix=DAP):
//DAP-SEND: {"command":"initialize"}
//DAP-SEND: {"command":"launch","arguments":{"program":"this-program-does-not-exist.slang"}}
//DAP-CHECK: "command" : "launch"
//DAP-CHECK: "success" : false
//DAP-CHECK: "message" : "Cannot open program:
//DAP-SEND: {"command":"disconnect"}
//DAP-CHECK: "event" : "terminated"
