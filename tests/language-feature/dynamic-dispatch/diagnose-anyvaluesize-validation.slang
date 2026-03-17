// Verify that [anyValueSize] rejects values exceeding the maximum (0x7FFF = 32767).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

[anyValueSize(99999)]
/*CHECK:
 ^^^^^^^^^^^^ 'anyValueSize' exceeds limit
 ^^^^^^^^^^^^ 'anyValueSize' cannot exceed 32767
*/
interface IFoo
{
    int get();
}
