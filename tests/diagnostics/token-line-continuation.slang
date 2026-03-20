//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Check the diagnostic output if a token uses \ line split

void foo() { br\
//diag:      ^^ undefined identifier
//diag:      ^^ undefined identifier 'br'.
eak; }
