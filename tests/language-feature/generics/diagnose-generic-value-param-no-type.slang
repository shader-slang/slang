//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Generic value parameters in functions must have an explicit type annotation.

void useValue<let N>(int x)
//CHECK:          ^ a generic value parameter must be given an explicit type
//CHECK:          ^ generic value parameter type 'error' is not supported; only integer and enum types are allowed
{
}

void main()
{
    useValue<4>(1);
}
