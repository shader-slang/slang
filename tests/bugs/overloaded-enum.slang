//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive):

enum E
{
    A,
/*diag:
    ^ see previous declaration of 'A'
*/
    A
/*diag:
    ^ conflicting declaration
    ^ declaration of 'A' conflicts with existing declaration
*/
}

E e = E::A;
//diag:  ^ ambiguous reference to 'A'
