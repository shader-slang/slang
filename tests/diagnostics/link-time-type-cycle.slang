//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): tests/diagnostics/link-time-type-cycle-a.slang tests/diagnostics/link-time-type-cycle-b.slang -target spirv

public extern struct A;
public extern struct B;
uniform A data;
[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}

/*CHECK:
cyclic reference 'A'
*/
