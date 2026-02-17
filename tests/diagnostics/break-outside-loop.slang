//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
//DIAGNOSTIC_TEST:COMMAND_LINE_SIMPLE(diag=diag):

// `break` where it isn't allowed

void foo() { break; }
//diag:      ^ 'break' must appear inside loop or switch constructs
//diag:      ^ 'break' must appear inside loop or switch constructs.
