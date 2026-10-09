// `__ref` and `__ref_readonly` are different passing modes, so a declaration and a definition
// that differ only between them declare two functions, and a call that both accept is ambiguous.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

int read(__ref_readonly int x);
/*CHECK:
    ^^^^ candidate: func read(const int) -> int
*/
int read(__ref int x) { return x + 1; }
/*CHECK:
    ^^^^ candidate: func read(int) -> int
*/

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    int a = 41;
    outputBuffer[0] = read(a);
//CHECK:                  ^ ambiguous call to 'read' with arguments of type (int)
}
