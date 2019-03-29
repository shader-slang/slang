// enum-implicit-conversion.slang

//DIAGNOSTIC_TEST:SIMPLE:

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

int test(int val)
{
    // Implicit conversion from `int` to `enum` isn't allowed.
    Color c = val;

    // TODO: explicit conversion to `enum` type should be allowed.
//	Color cc = Color(val);


    // Implicit converion from `enum` to `int` isn't allowed.
	int  x = c;
    uint y = c;

    // Explicit converion is allowed.
    int  xx = int(c);
    uint yy = uint(c);

    // Call that expects implicit conversion should fail.
    int z = foo(c);

    // Call that has an explicit overload on `enum` type should succeed.
    int zz = bar(c);

    return x + y + z;
}
