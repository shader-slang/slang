// enum-implicit-conversion.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Confirm that suitable error messages are
// generated for code that relies on implicit
// conversion of integers to/from `enum` types.

enum Color
{
    Red,
    Green,
    Blue,
    Alpha,
}

int foo(int  x) { return x * 16; }
int foo(uint x) { return x * 256 * 16; }

int bar(Color x) { return int(x) * 256; }
int bar(int   x) { return x * 256 * 256; }
int bar(uint  x) { return x * 256 * 256 * 16; }

int baz(Color x) { return (int)x; }
int test(int val)
{
    // Implicit conversion from `int` to `enum` isn't allowed.
    Color c = val;
//CHECK:      ^^^ type mismatch in expression
//CHECK:      ^^^ expected an expression of type 'Color', got 'int'
//CHECK:      ^^^ explicit conversion from 'int' to 'Color' is possible

    // Implicit cast from enum to int types other than the tag type is not allowed.
    uint y = c;
//CHECK:     ^ type mismatch in expression
//CHECK:     ^ expected an expression of type 'uint', got 'Color'
//CHECK:     ^ explicit conversion from 'Color' to 'uint' is possible

    // Call that expects implicit conversion from int to enum shouldn't be allowed.
    int z = baz(5);
//CHECK:        ^ type mismatch in expression
//CHECK:        ^ expected an expression of type 'Color', got 'int'
//CHECK:        ^ explicit conversion from 'int' to 'Color' is possible

    // Call that has an explicit overload on `enum` type should succeed.
    int zz = bar(c);
    
	Color cc = Color(val);
    
    // Implicit converion from `enum` to `int` is allowed.
	int  x = c;

    // Explicit converion is allowed.
    int  xx = int(c);
    uint yy = uint(c);

    return x + y + z;
}
