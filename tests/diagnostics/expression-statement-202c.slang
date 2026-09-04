//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-std 202c

// Under Slang 202c the statement-like warnings (an inappropriate statement form, and a discarded
// non-`void` result) are raised to errors. The severity is chosen per emission at the diagnostic
// site by the language version, with no global override. The non-proper-type error is already an
// error at every version.

int compute() { return 1; }

[numthreads(1, 1, 1)]
void main()
{
    int a = 1;

    // An inappropriate statement form is an error under 202c. Its discarded `int` result also
    // draws the discarded-result diagnostic, itself now an error.
    a;
/*diag:
    ^ error E30073
    ^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
    ^ error E30074
    ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/

    // A discarded non-`void` result is an error under 202c.
    compute();
/*diag:
           ^ error E30074
           ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/
}
