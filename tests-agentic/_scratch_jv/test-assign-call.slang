//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
int foo() { return 1; }
void main()
{
    foo() = 7;
}
