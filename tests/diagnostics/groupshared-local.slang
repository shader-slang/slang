//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

groupshared int b;

int test()
{
    groupshared int a;
/*diag:
    ^^^^^^^^^^^ modifier not allowed
    ^^^^^^^^^^^ modifier 'groupshared' is not allowed here.
*/
}
