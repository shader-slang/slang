// `__ref_readonly` and `__ref_writeonly` restrict a parameter's passing mode, so they are not
// allowed on a field. Each keyword spells two modifiers, but it is reported once.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

struct F
{
    __ref_writeonly int f;
/*CHECK:
    ^^^^^^^^^^^^^^^ E31201
    ^^^^^^^^^^^^^^^ modifier '__ref_writeonly' is not allowed here.
*/
    __ref_readonly int g;
/*CHECK:
    ^^^^^^^^^^^^^^ E31201
    ^^^^^^^^^^^^^^ modifier '__ref_readonly' is not allowed here.
*/
}

[numthreads(1, 1, 1)]
void main()
{
}
