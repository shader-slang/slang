//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

// Passing storage to opaque assembly does not read its uninitialized contents.
float address_operand(out float output)
{
    __intrinsic_asm "opaque", &output;
}

// Passing an ordinary value still reads it, even though the same instruction can write pointers.
float value_operand()
{
    float value;
    __intrinsic_asm "opaque", value;
/*CHK:
    ^^^^^^^^^^^^^^^ use of uninitialized variable
    ^^^^^^^^^^^^^^^ use of uninitialized variable 'value'
*/
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() {}
