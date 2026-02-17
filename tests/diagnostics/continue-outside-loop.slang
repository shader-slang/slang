//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
// `continue` where it isn't allowed

void foo() { continue; }
//diag:      ^ 'continue' must appear inside loop constructs
//diag:      ^ 'continue' must appear inside loop constructs.
