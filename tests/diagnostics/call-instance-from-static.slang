// call-instance-from-static.slang

// Test that calling an instance method from a static method is an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target hlsl -entry main -stage compute

struct Test
{
	void instanceMethod()
	{}

	static void staticMethod()
	{
		instanceMethod();
/*diag:
  ^ type cannot refer to non-static member
  ^ type 'Test' cannot be used to refer to non-static member 'instanceMethod'
*/
	}
}

void main()
{
	Test.staticMethod();
}
