// A digit separator does not hide a digit that is invalid for the literal's base: both
// literals report E10003 (invalid digit for base-N literal), like `0b12` and `079`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int x;
    x = 0b1_2;
//CHECK:    ^ error E10003
    x = 07_9;
//CHECK:^^^^ '0' prefix indicates octal literal
//CHECK:   ^ error E10003
}
