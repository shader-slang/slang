//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default
//TEST:SIMPLE(filecheck=BYTECODE): -target hgbc -entry main

[noinline]
uint selectValue(uint selector)
{
    uint result = 0;
    switch (selector)
    {
    case 0:
        result = 10;
        break;
    case 1:
    case 2:
        result = 20;
        break;
    case 3:
        result = 30;
    case 4:
        result += 4;
        break;
    default:
        result = 99;
        break;
    }
    return result;
}

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    print(selectValue(zero));
    print(selectValue(zero + 1));
    print(selectValue(zero + 2));
    print(selectValue(zero + 3));
    print(selectValue(zero + 4));
    print(selectValue(zero + 5));
}

// CHECK: 10
// CHECK-NEXT: 20
// CHECK-NEXT: 20
// CHECK-NEXT: 34
// CHECK-NEXT: 4
// CHECK-NEXT: 99
// BYTECODE: OpBranchConditional
