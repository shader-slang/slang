//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Using `each` on a regular type parameter (not a type pack) should produce
// error 30103 because `each` requires a type pack or tuple operand.

void useEachOnRegularType<T>(expand each T values)
//CHECK:                            ^^^^ expected type pack after 'each'
//CHECK:                            ^^^^ expected a type pack or a tuple after 'each'.
{
}
