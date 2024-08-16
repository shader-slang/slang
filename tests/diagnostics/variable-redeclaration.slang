// variable-redeclaration.slang

//TEST:SIMPLE(filecheck=CHECK): -target hlsl


// This test confirms that the compiler produces
// suitable diagnostics when variables are redeclared
// in a given scope.

// Global variables, including shader parameters

static int gA;

static Texture2D gA;

// Local variables

int testLocalRedeclaration(int x)
{
	int y = x;
	int y = x;
}

int testLocalShadowing(int x)
{
	int y = x;
	{
		// Because this declaration is in an inner
		// scope it should shadow the existing `y`
		// rather than conflcit with it.
		//
		// TODO: It would be reasonable for the
		// compiler to warn on this sort of code.
		//
		int y = x;
	}
}

// Structure fields

struct S
{
	int f;
	float f;
}

// Function parameter list

int testParameterRedeclaration(
	int 	size,
	float 	size)
{
	return size;
}

//CHECK: tests/diagnostics/variable-redeclaration.slang(14): error 30200: declaration of 'gA' conflicts with existing declaration
//CHECK: tests/diagnostics/variable-redeclaration.slang(12): note: see previous declaration of 'gA'
//CHECK: tests/diagnostics/variable-redeclaration.slang(44): error 30200: declaration of 'f' conflicts with existing declaration
//CHECK: tests/diagnostics/variable-redeclaration.slang(43): note: see previous declaration of 'f'
//CHECK: tests/diagnostics/variable-redeclaration.slang(51): error 30200: declaration of 'size' conflicts with existing declaration
//CHECK: tests/diagnostics/variable-redeclaration.slang(50): note: see previous declaration of 'size'
//CHECK: tests/diagnostics/variable-redeclaration.slang(21): error 30200: declaration of 'y' conflicts with existing declaration
//CHECK: tests/diagnostics/variable-redeclaration.slang(20): note: see previous declaration of 'y'
//CHECK: tests/diagnostics/variable-redeclaration.slang(53): error 39999: ambiguous reference to 'size'
//CHECK: tests/diagnostics/variable-redeclaration.slang(51): note 39999: candidate: float size
//CHECK: tests/diagnostics/variable-redeclaration.slang(50): note 39999: candidate: int size