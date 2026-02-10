//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Test that use of empty statement after an `if` leads to a warning.

int foo(){
    int x;
    return x[0];
//diag      ^ invalid subscript expression
//diag      ^ no subscript declarations found for type 'int'
}
