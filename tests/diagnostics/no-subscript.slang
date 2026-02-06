//DIAGNOSTIC_TEST:SIMPLE(diag=foo):

// Test that use of empty statement after an `if` leads to a warning.

int foo(){
    int x;
    return x[0];
//foo       ^ no subscript operation found for type 'int'
//foo      ^ type 'int'
}
