//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-std 202c

// Under Slang 202c the expression-statement warnings (case 1 and case 2 of
// shader-slang/slang#12428) are raised to errors, without any version branching in the checking
// logic: the severity is overridden for the duration of module checking via
// `DiagnosticSink::overrideDiagnosticSeverity`. Case 3 is already an error at every version.

int compute() { return 1; }

[numthreads(1, 1, 1)]
void main()
{
    int a = 1;

    // Case (1) (disallowed statement form) is an error under 202c. Its discarded `int` result also
    // draws the case-(2) diagnostic, itself now an error.
    a;
/*diag:
    ^ error E30073
    ^ the result of this expression is not used; did you mean to call it with '()'?
    ^ error E30074
    ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/

    // Case (2) (discarded non-`void` result) is an error under 202c.
    compute();
/*diag:
           ^ error E30074
           ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/
}
