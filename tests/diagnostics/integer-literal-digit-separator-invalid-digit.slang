// A digit separator does not hide a digit that is invalid for the literal's base.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int x;
    x = 0b1_2;
//CHECK:    ^ invalid digit for base-2 literal: '2'
    x = 07_9;
//CHECK:^^^^ '0' prefix indicates octal literal
//CHECK:   ^ invalid digit for base-8 literal: '9'
}
