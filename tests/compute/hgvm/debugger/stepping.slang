//TEST:DEBUGGER(mode=CLI, prefix=CLI):
//TEST:DEBUGGER(mode=CLI, prefix=INS):

[noinline]
int increment(int x) { return x + 1; }
[ForceInline]
int twice(int x) { return increment(x) + increment(x); }
[host]
void main()
{
    // Step into a physical call, finish it, and step over its inline caller.
    //CLI-SEND: start
    //CLI-CHECK: #0 twice (inlined) at
    //CLI-SEND: s
    //CLI-CHECK: #0 increment at
    //CLI-SEND: bt
    //CLI-CHECK: #0 increment at
    //CLI-CHECK-NEXT: #1 twice (inlined) at
    //CLI-CHECK-NEXT: #2 main at
    //CLI-SEND: finish
    //CLI-CHECK: #0 twice (inlined) at
    //CLI-SEND: n
    //CLI-CHECK: #0 main at {{.*}}stepping.slang:[[@LINE+1]]:
    print(twice(2));
    //CLI-SEND: n
    //CLI-CHECK: #0 main at {{.*}}stepping.slang:[[@LINE+1]]:
    print(increment(10));
}

//CLI-SEND: c
//CLI-CHECK: Program exited normally.
//CLI-SEND: q
//CLI-CHECK: standard output = {
//CLI-CHECK-NEXT: 6
//CLI-CHECK-NEXT: 11
//CLI-CHECK-NEXT: }

// nexti steps over a physical call; stepi enters it. An empty SEND repeats the previous command.
//INS-SEND: starti
//INS-CHECK: #0 twice (inlined) at
//INS-SEND: ni
//INS-CHECK: #0 twice (inlined) at
//INS-SEND: si
//INS-CHECK: #0 increment at
//INS-SEND:
//INS-CHECK: #0 increment at
//INS-SEND: finish
//INS-CHECK: #0 twice (inlined) at
//INS-SEND: c
//INS-CHECK: Program exited normally.
//INS-SEND: q
//INS-CHECK: standard output = {
//INS-CHECK-NEXT: 6
//INS-CHECK-NEXT: 11
//INS-CHECK-NEXT: }
