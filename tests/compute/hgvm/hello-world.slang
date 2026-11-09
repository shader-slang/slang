//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[host]
void main()
{
    print("Hello from HGVM");
}

// CHECK: Hello from HGVM
