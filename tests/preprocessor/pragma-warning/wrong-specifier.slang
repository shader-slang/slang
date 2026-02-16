//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
// Test that #pragma warning emits a warning if the specifier isn't correct

#pragma warning (diasble : 30081)
//diag:          ^ unknown pragma warning specifier
//diag:          ^ Unknown #pragma warning specifier 'diasble'
