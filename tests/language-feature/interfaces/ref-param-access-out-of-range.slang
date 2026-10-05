// A constant access on `RefParam` that is not an `Access` value is diagnosed,
// whether the type names a parameter or a struct field.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

void f(RefParam<int, (Access)7> x) {}
/*CHECK:
       ^^^^^^^^ E30034
       ^^^^^^^^ '7' is not a valid access for 'ref int'; expected 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'.
*/

struct S
{
    RefParam<float, (Access)7> p;
/*CHECK:
    ^^^^^^^^ E30034
    ^^^^^^^^ '7' is not a valid access for 'ref float'; expected 'Access.ReadWrite', 'Access.Read' or 'Access.Immutable'.
*/
}
