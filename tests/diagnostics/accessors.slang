// properties.slang

// Diagnostic messages related to property/subscript accessor declarations.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

struct Test
{
	property p : int
	{
		get(a) { return 0; }
/*diag:
  ^^^ accessors other than 'set' must not have parameters
      ^ a parameter declaration without an initial-value expression must be given an explicit type
      ^ a parameter declaration without an initial-value expression must be given an explicit type
*/
		set(a, b) { }
//diag:  ^ a 'set' accessor may not have more than one parameter
//diag:  ^ a parameter declaration without an initial-value expression must be given an explicit type
//diag:  ^ a parameter declaration without an initial-value expression must be given an explicit type
	}

	property q : float
	{
		set(c : int) { }
/*diag:
      ^ 'set' parameter 'c' has type 'int' which does not match the expected type 'float'
*/
	}
}
