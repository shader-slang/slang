//TEST:DEBUGGER(mode=CLI, prefix=QUIT):
//TEST:DEBUGGER(mode=CLI, prefix=RESTART):
//TEST:DEBUGGER(mode=CLI, prefix=EOF):

[noinline]
int value() { return 6; }
[host]
void main()
{
    print(value());
    print(11);
}

// Quit from inside a call must cancel without executing the remaining program.
//QUIT-SEND: starti
//QUIT-CHECK: #0 main at
//QUIT-SEND: si
//QUIT-CHECK: #0 value at
//QUIT-SEND: q
//QUIT-CHECK-NOT: Program exited normally.
//QUIT-CHECK: standard output = {
//QUIT-CHECK-NEXT: }

// Restarting at a stop cancels that invocation and executes one complete new invocation.
//RESTART-SEND: start
//RESTART-CHECK: #0 main at
//RESTART-SEND: run
//RESTART-CHECK: Program exited normally.
//RESTART-SEND: q
//RESTART-CHECK: standard output = {
//RESTART-CHECK-NEXT: 6
//RESTART-CHECK-NEXT: 11
//RESTART-CHECK-NEXT: }

// The runner closes stdin after the final SEND, exercising EOF at the entry stop.
//EOF-SEND: start
//EOF-CHECK: #0 main at
//EOF-CHECK-NOT: Program exited normally.
//EOF-CHECK: standard output = {
//EOF-CHECK-NEXT: }
