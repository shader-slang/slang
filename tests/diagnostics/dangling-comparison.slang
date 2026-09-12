//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

[numthreads(1, 1, 1)]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int a = 1;
    a == 2; // warn
/*diag:
      ^^ expression cannot be used as a statement
      ^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
      ^^ result of '==' not used
      ^^ result of '==' not used, did you intend '='?
*/
    // A comparison is not an allowed statement form, and parentheses are transparent to these
    // checks, so `(a == 2)` behaves exactly like the bare `a == 2` above: both draw the syntactic
    // form warning (E30073) and the `==`-specific result warning (E30058, "did you intend '='?")
    // rather than the generic discarded-result warning. They differ only in where E30073 points:
    // the bare form underlines the `==`, the parenthesized form the whole `(a == 2)` expression.
    (a == 2);
/*diag:
    ^ expression cannot be used as a statement
    ^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
       ^^ result of '==' not used
       ^^ result of '==' not used, did you intend '='?
*/
}
