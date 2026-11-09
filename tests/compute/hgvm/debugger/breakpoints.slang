//TEST:DEBUGGER(mode=CLI, prefix=CLI):
//TEST:DEBUGGER(mode=CLI, prefix=CLI, precompile=2):
//TEST:DEBUGGER(mode=CLI, prefix=RAW, precompile=0):
//TEST:DEBUGGER(mode=CLI, prefix=EDIT):
//TEST:DEBUGGER(mode=CLI, prefix=SOURCE):

[noinline]
int increment(int x) { return x + 1; }

[ForceInline]
int twice(int x) { return increment(x) + increment(x + 1); }

[host]
void main()
{
    print(twice(2));
    //SOURCE-SEND: b '$file':[[@LINE+1]]
    print(increment(10));
    //SOURCE-CHECK: Breakpoint 1, #0 main at {{.*}}:[[@LINE-1]]:
}

// A logical stack includes twice even though its instructions live in main.
// The same commands and checks run against source and precompiled HGBC with debug information.
//CLI-CHECK: result code = 0
//CLI-SEND: break increment
//CLI-CHECK: Breakpoint 1 (1 locations).
//CLI-SEND: info breakpoints
//CLI-CHECK: Num     Type           Disp Enb Address    What
//CLI-CHECK-NEXT: 1       breakpoint     keep y   0x{{([0-9a-f]{8})}} in increment at
//CLI-SEND: run
//CLI-CHECK: Breakpoint 1, #0 increment at
//CLI-SEND: bt
//CLI-CHECK: #0 increment at
//CLI-CHECK-NEXT: #1 twice (inlined) at
//CLI-CHECK-NEXT: #2 main at
//CLI-SEND: continue
//CLI-CHECK: Breakpoint 1, #0 increment at
//CLI-SEND: bt
//CLI-CHECK: #0 increment at
//CLI-CHECK-NEXT: #1 twice (inlined) at
//CLI-CHECK-NEXT: #2 main at
//CLI-SEND: c
//CLI-CHECK: Breakpoint 1, #0 increment at
//CLI-SEND: bt
//CLI-CHECK: #0 increment at
//CLI-CHECK-NEXT: #1 main at
//CLI-CHECK-NOT: #2
//CLI-SEND: c
//CLI-CHECK-NOT: Breakpoint 1,
//CLI-CHECK: Program exited normally.
//CLI-SEND: quit
//CLI-CHECK: standard output = {
//CLI-CHECK-NEXT: 7
//CLI-CHECK-NEXT: 11
//CLI-CHECK-NEXT: }

// With -g0, function breakpoints still work, and no source or inline frames are invented.
//RAW-SEND: b increment
//RAW-SEND: info breakpoints
//RAW-CHECK: Num Type Disp Enb Address What
//RAW-CHECK-NEXT: 1 breakpoint keep y 0x{{([0-9a-f]{8})}} in increment{{$}}
//RAW-SEND: r
//RAW-CHECK: Breakpoint 1, #0 increment{{$}}
//RAW-SEND: bt
//RAW-CHECK: #0 increment{{$}}
//RAW-CHECK-NEXT: #1 main{{$}}
//RAW-CHECK-NOT: #2
//RAW-SEND: c
//RAW-CHECK: Breakpoint 1, #0 increment{{$}}
//RAW-SEND: c
//RAW-CHECK: Breakpoint 1, #0 increment{{$}}
//RAW-SEND: c
//RAW-CHECK: Program exited normally.
//RAW-SEND: q
//RAW-CHECK: standard output = {
//RAW-CHECK-NEXT: 7
//RAW-CHECK-NEXT: 11
//RAW-CHECK-NEXT: }

// Disabling and deleting a breakpoint must remove its armed bits on subsequent runs.
//EDIT-SEND: b increment
//EDIT-CHECK: Breakpoint 1 (1 locations).
//EDIT-SEND: disable 1
//EDIT-SEND: r
//EDIT-CHECK-NOT: Breakpoint 1,
//EDIT-CHECK: Program exited normally.
//EDIT-SEND: enable 1
//EDIT-SEND: delete 1
//EDIT-SEND: r
//EDIT-CHECK-NOT: Breakpoint 1,
//EDIT-CHECK: Program exited normally.
//EDIT-SEND: q
//EDIT-CHECK: standard output = {
//EDIT-CHECK-NEXT: 7
//EDIT-CHECK-NEXT: 11
//EDIT-CHECK-NEXT: 7
//EDIT-CHECK-NEXT: 11
//EDIT-CHECK-NEXT: }

// The source breakpoint command is placed next to its target statement above.
//SOURCE-SEND: r
//SOURCE-SEND: bt
//SOURCE-CHECK: #0 main at
//SOURCE-SEND: c
//SOURCE-CHECK: Program exited normally.
//SOURCE-SEND: q
//SOURCE-CHECK: standard output = {
//SOURCE-CHECK-NEXT: 7
//SOURCE-CHECK-NEXT: 11
//SOURCE-CHECK-NEXT: }
