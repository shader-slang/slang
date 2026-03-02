//DIAGNOSTIC_TEST(smoke):SIMPLE(diag=CHK):
// call function with wrong argument type

struct A {};
struct B {};

void f(A a) {}
void g(B b)
{
	f(b);
/*CHK:
   ^ type mismatch in expression
   ^ expected an expression of type 'A', got 'B'
*/
}
