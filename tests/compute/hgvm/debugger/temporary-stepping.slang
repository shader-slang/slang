//TEST:DEBUGGER(mode=CLI, prefix=CLI):
//TEST:DEBUGGER(mode=CLI, prefix=CLI, precompile=2):

[noinline]
int work(int x)
{
    print(x);
    return x + 1;
}

[ForceInline]
int inner(int x)
{
    //CLI-SEND: break '$file':[[@LINE+1]]
    if (x > 0) return work(x);
    //CLI-SEND: break '$file':[[@LINE+1]]
    return work(-x);
}

[ForceInline]
int outer(int x)
{
    int a = inner(x);
    int b = inner(-x);
    return a + b;
}

[host]
void main()
{
    int sum = 0;
    for (int i = 0; i < 2; ++i)
        sum += outer(i + 1);
    print(sum);
    print(work(10));
}

// Finishing inner resumes outer. A user breakpoint in the second inline instance must interrupt
// finish even though that instance is deeper than the requested destination.
//CLI-SEND: run
//CLI-CHECK: Breakpoint 1, #0 inner (inlined) at
//CLI-SEND: disable 1
//CLI-SEND: finish
//CLI-CHECK: #0 outer (inlined) at
//CLI-SEND: finish
//CLI-CHECK: Breakpoint 2, #0 inner (inlined) at
//CLI-SEND: delete 2
//CLI-SEND: finish
//CLI-CHECK: #0 outer (inlined) at
//CLI-SEND: finish
//CLI-CHECK: #0 main at
//CLI-SEND: continue
//CLI-CHECK: Program exited normally.
// Restart must not encounter temporary breakpoints from any of the previous finish commands.
//CLI-SEND: run
//CLI-CHECK-NOT: #0
//CLI-CHECK: Program exited normally.
//CLI-SEND: quit
//CLI-CHECK: standard output = {
//CLI-CHECK-NEXT: 1
//CLI-CHECK-NEXT: 1
//CLI-CHECK-NEXT: 2
//CLI-CHECK-NEXT: 2
//CLI-CHECK-NEXT: 10
//CLI-CHECK-NEXT: 10
//CLI-CHECK-NEXT: 11
//CLI-CHECK-NEXT: 1
//CLI-CHECK-NEXT: 1
//CLI-CHECK-NEXT: 2
//CLI-CHECK-NEXT: 2
//CLI-CHECK-NEXT: 10
//CLI-CHECK-NEXT: 10
//CLI-CHECK-NEXT: 11
//CLI-CHECK-NEXT: }
