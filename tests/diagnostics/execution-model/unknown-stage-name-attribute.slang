//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[shader("invalid")]
/*CHECK:
 ^^^^^^ unknown capability
 ^^^^^^ unknown capability name 'invalid'.
*/
void main() {}
