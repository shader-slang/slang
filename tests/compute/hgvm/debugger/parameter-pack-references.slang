//TEST:DEBUGGER(mode=CLI, prefix=CLI):

[noinline]
void update<each T>(Tuple<expand each T> source, __ref expand each T values)
{
    values = expand each source;
    //CLI-SEND: break '$file':[[@LINE+1]]
    print(uint(123));
}

[host]
void main()
{
    var values = makeTuple(uint(0), 0.0);
    update(makeTuple(uint(7), 3.0), expand each values);
}

//CLI-SEND: run
//CLI-CHECK: Breakpoint 1, #0 update
//CLI-SEND: info args
//CLI-CHECK: values = Tuple {{.*}}= 7, {{.*}}= 3}
//CLI-SEND: p values
//CLI-CHECK: $1 = Tuple {{.*}}= 7, {{.*}}= 3}
//CLI-SEND: continue
//CLI-CHECK: Program exited normally.
//CLI-SEND: quit
