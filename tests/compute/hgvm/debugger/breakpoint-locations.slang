//TEST:DEBUGGER(mode=CLI, prefix=CLI):
//TEST:DEBUGGER(mode=CLI, prefix=CLI, precompile=2):

// One source breakpoint resolves to both inline copies of this statement.
[ForceInline]
int bump(int x)
{
    //CLI-SEND: break '$file':[[@LINE+1]]
    return x + 1;
}

[noinline]
int first(int x) { return bump(x); }
[noinline]
int second(int x) { return bump(x * 2); }

[host]
void main()
{
    print(first(2));
    print(second(4));
}

//CLI-CHECK: Breakpoint 1 (2 locations).
//CLI-SEND: break main
//CLI-CHECK: Breakpoint 2 (1 locations).
//CLI-SEND: break missingFunction
//CLI-CHECK: Breakpoint 3 (0 locations, pending).
//CLI-SEND: info breakpoints
//CLI-CHECK: Num Type Disp Enb Address What
//CLI-CHECK-NEXT: 1 breakpoint keep y <MULTIPLE>
//CLI-CHECK-NEXT: 1.1 y [[FIRST:0x[0-9a-f]{8}]] in bump at [[SOURCE:.*breakpoint-locations.slang:[0-9]+]]
//CLI-CHECK-NEXT: 1.2 y [[SECOND:0x[0-9a-f]{8}]] in bump at [[SOURCE]]
//CLI-CHECK-NEXT: 2 breakpoint keep y [[MAIN:0x[0-9a-f]{8}]] in main at
//CLI-CHECK-NEXT: 3 breakpoint keep y <PENDING> missingFunction

// Location numbers and addresses must survive changes to the parent's enabled state.
//CLI-SEND: disable 1
//CLI-SEND: info b
//CLI-CHECK: Num Type Disp Enb Address What
//CLI-CHECK-NEXT: 1 breakpoint keep n <MULTIPLE>
//CLI-CHECK-NEXT: 1.1 n [[FIRST]] in bump at [[SOURCE]]
//CLI-CHECK-NEXT: 1.2 n [[SECOND]] in bump at [[SOURCE]]
//CLI-CHECK-NEXT: 2 breakpoint keep y [[MAIN]] in main at
//CLI-CHECK-NEXT: 3 breakpoint keep y <PENDING> missingFunction
//CLI-SEND: enable 1
//CLI-SEND: delete 2
//CLI-SEND: delete 3
//CLI-SEND: run
//CLI-CHECK: Breakpoint 1, #0 bump (inlined) at
//CLI-SEND: continue
//CLI-CHECK: Breakpoint 1, #0 bump (inlined) at
//CLI-SEND: continue
//CLI-CHECK: Program exited normally.
//CLI-SEND: delete
//CLI-SEND: info break
//CLI-CHECK: No breakpoints or watchpoints.
//CLI-SEND: quit
//CLI-CHECK: standard output = {
//CLI-CHECK-NEXT: 3
//CLI-CHECK-NEXT: 9
//CLI-CHECK-NEXT: }
