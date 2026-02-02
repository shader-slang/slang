// properties.slang

// Diagnostic messages related to property/subscript accessor declarations.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct Test
{
	property p : int
	{
		//CHECK: ([[# @LINE+1]]): error 31101: accessors other than 'set' must not have parameters
		get(a) { return 0; }
		//CHECK: ([[# @LINE+1]]): error 31102: a 'set' accessor may not have more than one parameter
		set(a, b) { }
	}

	property q : float
	{
		//CHECK: ([[# @LINE+1]]): error 31102: 'set' parameter 'c' has type 'int' which does not match the expected type 'float'
		set(c : int) { }
	}
}