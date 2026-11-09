//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
void main()
{
    hgbc_asm {
        OpWindowType %windowType;
        // CHECK: error[E29120]
        // CHECK: OpWindowType is compiler-owned and cannot be written in hgbc_asm
    };
}
