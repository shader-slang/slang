//DIAGNOSTIC_TEST:SIMPLE(diag=diag):


struct Color
{
	int Red;
}

void test()
{
	int x = Color.Red;
//diag:        ^ type cannot refer to non-static member
//diag:        ^ type 'Color' cannot be used to refer to non-static member 'Red'
}
