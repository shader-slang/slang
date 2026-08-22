//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int a = 1;
    a == 2; // warn
/*diag:
      ^^ this form of expression should not be used as a statement
      ^^ the result of this expression is not used; did you mean to call it with '()'?
      ^^ result of '==' not used
      ^^ result of '==' not used, did you intend '='?
*/
    // A comparison is not one of the allowed statement forms, and (once parentheses are peeled) its
    // `bool` result is discarded, so it now draws the syntactic (E30073) and discarded-result
    // (E30074) diagnostics of shader-slang/slang#12428. The `did you intend '='?` heuristic (E30058)
    // only fires on the unparenthesized form.
    (a == 2);
/*diag:
       ^^ this form of expression should not be used as a statement
       ^^ the result of this expression is not used; did you mean to call it with '()'?
       ^^ result of this expression is not used
       ^^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/
}
