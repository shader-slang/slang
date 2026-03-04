//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

int foo(){
    int x;
    return x[0];
//diag:     ^ invalid subscript expression
//diag:     ^ no subscript declarations found for type 'int'
}
