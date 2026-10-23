//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -std legacy
//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -std 2025
//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -std 2026
//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -std 202c

Tuple t0;
/*diag:
^^^^^ generic type 'Tuple' used without argument
*/

// This should be OK, compiling down to a zero-sized type
Tuple<> t1;

// A bare generic name fills declared defaults, but does not infer missing arguments from
// constraints. Explicit specialization can still use the same constraint.
struct Fixed<T> where T == int
{
    T value;
}
Fixed missingDefault;
/*diag:
^^^^^ generic type 'Fixed' used without argument
*/
Fixed<int> explicitArgument;

[numthreads(1,1,1)]
void computeMain()
{
}
