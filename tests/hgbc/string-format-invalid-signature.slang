//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

String invalidFormat()
{
    return hgbc_asm {
        result : $$String = OpFormatString;
        // CHECK: error[E29123]
        // CHECK: OpFormatString has an invalid number of operands
    };
}

[host]
void main()
{
    print(invalidFormat());
}
