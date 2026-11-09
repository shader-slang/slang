//TEST:DEBUGGER(mode=CLI, prefix=CLI):

[host]
void main()
{
    //CLI-SEND: start
    //CLI-CHECK: #0 main at {{.*}}temporary-stepping-loop.slang:[[LOOP:[0-9]+]]:
    int i = 0;
    // Keep the loop on one line: each backward edge should still finish one next command.
    while (i < 3) { i++; }
    //CLI-SEND: p i
    //CLI-CHECK: $1 = 0
    //CLI-SEND: next
    //CLI-CHECK: #0 main at {{.*}}temporary-stepping-loop.slang:[[LOOP]]:
    //CLI-SEND: p i
    //CLI-CHECK: $2 = 1
    //CLI-SEND: next
    //CLI-CHECK: #0 main at {{.*}}temporary-stepping-loop.slang:[[LOOP]]:
    //CLI-SEND: p i
    //CLI-CHECK: $3 = 2
    //CLI-SEND: next
    //CLI-CHECK: #0 main at {{.*}}temporary-stepping-loop.slang:[[LOOP]]:
    //CLI-SEND: p i
    //CLI-CHECK: $4 = 3
    //CLI-SEND: next
    //CLI-CHECK: #0 main at {{.*}}temporary-stepping-loop.slang:[[@LINE+1]]:
    print(i);
}

//CLI-SEND: continue
//CLI-CHECK: Program exited normally.
//CLI-SEND: quit
//CLI-CHECK: standard output = {
//CLI-CHECK-NEXT: 3
//CLI-CHECK-NEXT: }
