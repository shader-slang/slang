//TEST:INTERPRET(hgvm,filecheck=CHECK):
//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[host]
void main()
{
    print("HGVM test-server output");
}

// CHECK: HGVM test-server output
