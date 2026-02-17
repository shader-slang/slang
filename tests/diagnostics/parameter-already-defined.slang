//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
// re-use parameter name

int foo( int a, float a ) { return 0; }
//diag:               ^ conflicting declaration
//diag:               ^ declaration of 'a' conflicts with existing declaration
//diag:      ^ see previous declaration of 'a'
