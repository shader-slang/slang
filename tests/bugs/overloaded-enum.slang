//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

enum E
{
    A,
/*diag:
    ^ see previous declaration of 'A'
    ^ candidate: E E.A
*/
    A
/*diag:
    ^ conflicting declaration
    ^ declaration of 'A' conflicts with existing declaration
    ^ candidate: E E.A
*/
}

E e = E::A;
//diag:  ^ ambiguous reference to 'A'
//diag:  ^ ambiguous reference to 'A'
