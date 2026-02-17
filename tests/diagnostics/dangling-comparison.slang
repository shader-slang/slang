//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int a = 1;
    a == 2; // warn
/*diag:
      ^^ result of '==' not used
      ^^ result of '==' not used, did you intend '='?
*/
    (a == 2); // ok.
}
