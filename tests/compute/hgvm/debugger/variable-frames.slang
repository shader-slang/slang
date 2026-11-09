//TEST:DEBUGGER(mode=CLI, prefix=CLI):

[noinline]
uint leaf(__ref uint reference, uint input)
{
    uint local = input + 1;
    reference += 2;
    //CLI-SEND: break '$file':[[@LINE+1]]
    print(local);
    return local;
}

[ForceInline]
uint inner(uint input)
{
    uint local = input + 10;
    {
        uint local = input + 20;
        return leaf(local, input);
    }
}

[host]
void main()
{
    uint local = 100;
    print(inner(3));
    print(local);
    print(inner(7));
}

//CLI-SEND: run
//CLI-CHECK: Breakpoint 1, #0 leaf at
//CLI-SEND: info args
//CLI-CHECK-DAG: reference = 25
//CLI-CHECK-DAG: input = 3
//CLI-SEND: info locals
//CLI-CHECK: local = 4
//CLI-SEND: p *reference
//CLI-CHECK: $1 = 25
//CLI-SEND: up
//CLI-CHECK: #1 inner (inlined) at
//CLI-SEND: info args
//CLI-CHECK: input = 3
//CLI-SEND: p local
//CLI-CHECK: $2 = 23
//CLI-SEND: up
//CLI-CHECK: #2 main at
//CLI-SEND: p local
//CLI-CHECK: $3 = 100
//CLI-SEND: continue
//CLI-CHECK: Breakpoint 1, #0 leaf at
//CLI-SEND: info args
//CLI-CHECK-DAG: reference = 29
//CLI-CHECK-DAG: input = 7
//CLI-SEND: frame 1
//CLI-CHECK: #1 inner (inlined) at
//CLI-SEND: info args
//CLI-CHECK: input = 7
//CLI-SEND: p local
//CLI-CHECK: $4 = 27
//CLI-SEND: continue
//CLI-CHECK: Program exited normally.
//CLI-SEND: quit
