// Test that using an unknown stage name in [shader(...)] attribute produces an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[shader("invalid")]
/*CHECK:
 ^^^^^^ unknown capability
 ^^^^^^ unknown capability name 'invalid'.
*/
void main()
{}
