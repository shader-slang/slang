//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):
// bad type for `while` predicate

struct S {};

void foo()
{
	S s;
	while(s) {break;}
//CHK: ^ type mismatch in expression
//CHK: ^ expected an expression of type 'bool', got 'S'
}
