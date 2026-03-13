//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void foo() {
// Note: no closing curly brace
//CHECK: unexpected token
//CHECK: unexpected end of file, expected '}'
