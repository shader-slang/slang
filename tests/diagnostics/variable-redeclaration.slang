// variable-redeclaration.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):


// This test confirms that the compiler produces
// suitable diagnostics when variables are redeclared
// in a given scope.

// Global variables, including shader parameters

static int gA;
//diag:    ^^ see previous declaration of 'gA'

static Texture2D gA;
//diag:          ^^ conflicting declaration
//diag:          ^^ declaration of 'gA' conflicts with existing declaration
//diag:          ^^ global variable cannot have opaque type
//diag:          ^^ global variable cannot have opaque type.
//diag:          ^^ do you intend to define a `uniform` parameter instead?

// Local variables

int testLocalRedeclaration(int x)
{
	int y = x;
/*diag:
     ^ see previous declaration of 'y'
*/
	int y = x;
/*diag:
     ^ conflicting declaration
     ^ declaration of 'y' conflicts with existing declaration
*/
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
// Note: more diagnostics will be reported here because of the constructor synthesis
struct S
//diag:^ conflicting declaration
//diag:^ declaration of 'f' conflicts with existing declaration
//diag:^ see previous declaration of 'f'
{
	int f;
/*diag:
     ^ see previous declaration of 'f'
     ^ ambiguous reference to 'f'
     ^ candidate: int S.f
     ^ candidate: int S.f
*/
	float f;
//diag:^ conflicting declaration
//diag:^ declaration of 'f' conflicts with existing declaration
//diag:^ candidate: float S.f
//diag:^ ambiguous reference to 'f'
//diag:^ candidate: float S.f
}

// Function parameter list

int testParameterRedeclaration(
	int 	size,
/*diag:
      ^^^^ see previous declaration of 'size'
      ^^^^ candidate: int size
*/
	float 	size)
//diag: ^^^^ conflicting declaration
//diag: ^^^^ declaration of 'size' conflicts with existing declaration
//diag: ^^^^ candidate: float size
{
	return size;
/*diag:
        ^^^^ ambiguous reference to 'size'
*/
}
