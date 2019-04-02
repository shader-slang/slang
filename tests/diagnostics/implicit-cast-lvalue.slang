//DIAGNOSTIC_TEST:SIMPLE:

// Passing an argument for an `out` parameter such
// that implicit conversion would be required in
// both directions.

void a(out uint x)
{
	x = 0;
}

void b(int y)
{
	a(y);
}
