// Check that a misspelled `spirv_asm` qualifier is reported as an unexpected identifier, naming the
// qualifier that is accepted.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    uint value = spirv_asm [__assumeValidResourceOperand]
//CHECK:                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ E20004
//CHECK:                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ unexpected identifier, expected '__assumeValidResourceOperands'
    {
        result:$$uint = OpConstant 123;
    };
}
